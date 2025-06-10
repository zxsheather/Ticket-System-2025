const express = require('express');
const session = require('express-session');
const bodyParser = require('body-parser');
const cors = require('cors');
const path = require('path');
const http = require('http');
const socketIo = require('socket.io');
const winston = require('winston');
const { v4: uuidv4 } = require('uuid');
const helmet = require('helmet');
const compression = require('compression');
const rateLimit = require('express-rate-limit');

// 导入路由模块
const userRoutes = require('./routes/user');
const trainRoutes = require('./routes/train');
const ticketRoutes = require('./routes/ticket');
const orderRoutes = require('./routes/order');
const adminRoutes = require('./routes/admin');

// 导入中间件
const { requireAuth, requireAdmin, optionalAuth, redirectIfAuthenticated } = require('./middleware/auth');
const { notFoundHandler, errorHandler } = require('./middleware/error');
const { validateLogin, validateRegister, checkValidationResult } = require('./middleware/validation');

// 导入控制器和工具
const ticketController = require('./controllers/ticketController');
const systemMonitor = require('./utils/systemMonitor');
const notificationSystem = require('./utils/notificationSystem');

// 创建Express应用
const app = express();
const server = http.createServer(app);
const io = socketIo(server, {
  cors: {
    origin: "*",
    methods: ["GET", "POST"]
  }
});

// 配置日志
const logger = winston.createLogger({
  level: 'info',
  format: winston.format.combine(
    winston.format.timestamp(),
    winston.format.errors({ stack: true }),
    winston.format.json()
  ),
  defaultMeta: { service: 'ticket-web-frontend' },
  transports: [
    new winston.transports.File({ filename: 'logs/error.log', level: 'error' }),
    new winston.transports.File({ filename: 'logs/combined.log' }),
    new winston.transports.Console({
      format: winston.format.simple()
    })
  ]
});

// 中间件配置
app.use(helmet({
  contentSecurityPolicy: false // 开发环境暂时禁用CSP
}));

app.use(compression());

// 请求频率限制
const limiter = rateLimit({
  windowMs: 15 * 60 * 1000, // 15分钟
  max: 100, // 限制每个IP最多100个请求
  message: '请求过于频繁，请稍后再试'
});
app.use('/api/', limiter);

app.use(cors({
  origin: true,
  credentials: true
}));

app.use(bodyParser.json({ limit: '10mb' }));
app.use(bodyParser.urlencoded({ extended: true, limit: '10mb' }));

// 会话配置
app.use(session({
  secret: process.env.SESSION_SECRET || 'ticket-system-secret-key-2025',
  resave: false,
  saveUninitialized: false,
  cookie: {
    secure: false, // 在生产环境中应设为true（使用HTTPS）
    maxAge: 24 * 60 * 60 * 1000 // 24小时
  }
}));

// 静态文件服务
app.use(express.static(path.join(__dirname, 'public')));
app.use('/css', express.static(path.join(__dirname, 'public/css')));
app.use('/js', express.static(path.join(__dirname, 'public/js')));
app.use('/images', express.static(path.join(__dirname, 'public/images')));

// 设置视图引擎
app.set('view engine', 'ejs');
app.set('views', path.join(__dirname, 'views'));

// 健康检查端点
app.get('/health', (req, res) => {
  const healthStatus = {
    status: 'ok',
    timestamp: new Date().toISOString(),
    uptime: process.uptime(),
    version: require('./package.json').version || '1.0.0',
    environment: process.env.NODE_ENV || 'development',
    memory: process.memoryUsage(),
    pid: process.pid
  };
  
  // 检查系统监控器状态
  if (systemMonitor) {
    healthStatus.systemMetrics = systemMonitor.getMetrics();
  }
  
  res.status(200).json(healthStatus);
});

// 系统状态端点（管理员专用）
app.get('/api/system/status', requireAuth, (req, res) => {
  if (!req.session.user || req.session.user.privilege < 5) {
    return res.status(403).json({ error: '权限不足' });
  }
  
  const systemStatus = {
    application: {
      name: 'Ticket System 2025 Web Frontend',
      version: require('./package.json').version || '1.0.0',
      environment: process.env.NODE_ENV || 'development',
      uptime: process.uptime(),
      pid: process.pid
    },
    server: {
      platform: process.platform,
      arch: process.arch,
      nodeVersion: process.version,
      memory: process.memoryUsage()
    },
    timestamp: new Date().toISOString()
  };
  
  // 添加系统监控数据
  if (systemMonitor) {
    systemStatus.metrics = systemMonitor.getMetrics();
    systemStatus.alerts = systemMonitor.getAlerts();
  }
  
  res.json(systemStatus);
});

// 全局中间件 - 设置用户信息到本地变量
app.use((req, res, next) => {
  res.locals.user = req.session.user || null;
  res.locals.currentPage = req.path.slice(1) || 'home';
  next();
});

// 路由配置
app.use('/api/user', userRoutes);
app.use('/api/train', trainRoutes);
app.use('/api/ticket', ticketRoutes);
app.use('/api/order', orderRoutes);
app.use('/api/admin', adminRoutes);

// 主页路由
app.get('/', optionalAuth, (req, res) => {
  res.render('index', {
    title: '火车票管理系统',
    user: req.session.user || null
  });
});

// 登录页面
app.get('/login', redirectIfAuthenticated, (req, res) => {
  res.render('login', { 
    title: '用户登录',
    user: null 
  });
});

// 注册页面
app.get('/register', redirectIfAuthenticated, (req, res) => {
  res.render('register', { 
    title: '用户注册',
    user: null 
  });
});

// 车票查询页面
app.get('/search', optionalAuth, async (req, res) => {
  // 从查询参数中获取搜索条件
  const searchParams = {
    from: req.query.from || '',
    to: req.query.to || '',
    date: req.query.date || '',
    sort: req.query.sort || 'time'
  };
  
  let trains = [];
  let error = null;
  
  // 如果有搜索参数，执行搜索
  if (searchParams.from && searchParams.to && searchParams.date) {
    try {
      logger.info(`执行车票查询: ${searchParams.from} -> ${searchParams.to}, 日期: ${searchParams.date}`);
      const result = await ticketController.queryTicket(
        searchParams.from, 
        searchParams.to, 
        searchParams.date, 
        searchParams.sort
      );
      
      if (result.success && result.tickets) {
        trains = result.tickets.map(ticket => ({
          trainID: ticket.trainId,
          from: ticket.from,
          to: ticket.to,
          startTime: ticket.leavingTime,
          arrivalTime: ticket.arrivingTime,
          price: ticket.price,
          seat: ticket.seats,
          duration: calculateDuration(ticket.leavingTime, ticket.arrivingTime)
        }));
        logger.info(`找到 ${trains.length} 趟列车`);
      } else {
        error = '未找到符合条件的列车';
        logger.warn(`搜索无结果: ${JSON.stringify(searchParams)}`);
      }
    } catch (err) {
      error = '搜索失败，请重试';
      logger.error('车票查询失败:', err);
    }
  }
  
  res.render('search', {
    title: '车票查询',
    user: req.session.user || null,
    searchParams: searchParams,
    trains: trains,
    error: error
  });
});

// 计算旅行时间的辅助函数
function calculateDuration(startTime, endTime) {
  try {
    // 解析时间格式，假设格式为 "MM-DD HH:mm"
    const parseTime = (timeStr) => {
      const parts = timeStr.split(' ');
      if (parts.length !== 2) return null;
      
      const [datePart, timePart] = parts;
      const [month, day] = datePart.split('-').map(Number);
      const [hour, minute] = timePart.split(':').map(Number);
      
      const currentYear = new Date().getFullYear();
      return new Date(currentYear, month - 1, day, hour, minute);
    };
    
    const start = parseTime(startTime);
    const end = parseTime(endTime);
    
    if (!start || !end) return '计算中';
    
    const diffMs = end.getTime() - start.getTime();
    const diffHours = Math.floor(diffMs / (1000 * 60 * 60));
    const diffMinutes = Math.floor((diffMs % (1000 * 60 * 60)) / (1000 * 60));
    
    if (diffHours > 0) {
      return `${diffHours}小时${diffMinutes}分钟`;
    } else {
      return `${diffMinutes}分钟`;
    }
  } catch (err) {
    logger.warn('计算旅行时间失败:', err);
    return '计算中';
  }
}

// 订单管理页面
app.get('/orders', requireAuth, (req, res) => {
  res.render('orders', {
    title: '我的订单',
    user: req.session.user
  });
});

// 个人中心页面
app.get('/profile', requireAuth, (req, res) => {
  res.render('profile', {
    title: '个人资料',
    user: req.session.user
  });
});

// 管理员页面
app.get('/admin', requireAuth, requireAdmin, (req, res) => {
  res.render('admin', {
    title: '系统管理',
    user: req.session.user
  });
});

// 初始化WebSocket和监控系统
notificationSystem.initialize(io);
systemMonitor.startMonitoring(io);

// 导出WebSocket工具函数
app.sendNotificationToUser = (username, message, type = 'info') => {
  notificationSystem.sendNotificationToUser(username, {
    title: '系统通知',
    message: message,
    type: type,
    level: type
  });
};

app.broadcastNotification = (title, message, level = 'info') => {
  notificationSystem.broadcastNotification({
    title: title,
    message: message,
    level: level,
    type: 'system'
  });
};

// API路由 - 获取系统状态
app.get('/api/system/status', requireAuth, async (req, res) => {
  try {
    const health = await systemMonitor.getHealthStatus();
    const stats = notificationSystem.getSystemStats();
    
    res.json({
      success: true,
      data: {
        health: health,
        notifications: stats,
        timestamp: Date.now()
      }
    });
  } catch (error) {
    logger.error('获取系统状态失败:', error);
    res.status(500).json({
      success: false,
      message: '获取系统状态失败'
    });
  }
});

// API路由 - 获取用户通知
app.get('/api/notifications', requireAuth, (req, res) => {
  try {
    const notifications = notificationSystem.getUserNotifications(req.session.user.username);
    const unreadCount = notificationSystem.getUnreadCount(req.session.user.username);
    
    res.json({
      success: true,
      data: {
        notifications: notifications,
        unreadCount: unreadCount
      }
    });
  } catch (error) {
    logger.error('获取用户通知失败:', error);
    res.status(500).json({
      success: false,
      message: '获取通知失败'
    });
  }
});

// API路由 - 标记通知为已读
app.post('/api/notifications/:id/read', requireAuth, (req, res) => {
  try {
    const success = notificationSystem.markNotificationAsRead(
      req.session.user.username,
      req.params.id
    );
    
    if (success) {
      res.json({
        success: true,
        message: '通知已标记为已读'
      });
    } else {
      res.status(404).json({
        success: false,
        message: '通知不存在'
      });
    }
  } catch (error) {
    logger.error('标记通知已读失败:', error);
    res.status(500).json({
      success: false,
      message: '操作失败'
    });
  }
});

// 404和错误处理（必须放在所有路由之后）
app.use(notFoundHandler);
app.use(errorHandler);

// 优雅关闭处理
const gracefulShutdown = () => {
  logger.info('收到关闭信号，正在关闭服务器...');
  
  // 停止监控系统
  systemMonitor.stopMonitoring();
  
  // 通知所有用户系统即将维护
  notificationSystem.broadcastNotification({
    type: 'maintenance',
    title: '系统维护通知',
    message: '系统即将进行维护，请保存您的工作',
    level: 'warning'
  });
  
  server.close(() => {
    logger.info('HTTP服务器已关闭');
    
    // 关闭数据库连接等其他资源
    ticketController.cleanup();
    
    process.exit(0);
  });
  
  // 强制关闭
  setTimeout(() => {
    logger.error('强制关闭服务器');
    process.exit(1);
  }, 10000);
};

process.on('SIGTERM', gracefulShutdown);
process.on('SIGINT', gracefulShutdown);

// 启动服务器
const PORT = process.env.PORT || 3000;
server.listen(PORT, () => {
  logger.info(`Web前端服务器启动成功，端口: ${PORT}`);
  logger.info(`访问地址: http://localhost:${PORT}`);
});

module.exports = { app, server, io, logger };
