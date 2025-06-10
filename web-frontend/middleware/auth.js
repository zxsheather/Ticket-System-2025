// 认证中间件
const winston = require('winston');

const logger = winston.createLogger({
  level: 'info',
  format: winston.format.combine(
    winston.format.timestamp(),
    winston.format.json()
  ),
  defaultMeta: { service: 'auth-middleware' },
  transports: [
    new winston.transports.File({ filename: 'logs/auth.log' }),
    new winston.transports.Console({
      format: winston.format.simple()
    })
  ]
});

// 检查用户是否已登录
function requireAuth(req, res, next) {
  if (req.session && req.session.user) {
    logger.info(`用户 ${req.session.user.username} 通过认证检查`);
    return next();
  } else {
    logger.warn(`未登录用户尝试访问受保护的路由: ${req.path}`);
    
    if (req.xhr || req.headers.accept.indexOf('json') > -1) {
      // AJAX请求返回JSON
      return res.status(401).json({ 
        success: false, 
        message: '请先登录',
        redirect: '/login'
      });
    } else {
      // 普通请求重定向到登录页
      req.session.returnTo = req.originalUrl;
      return res.redirect('/login');
    }
  }
}

// 检查用户是否为管理员
function requireAdmin(req, res, next) {
  if (req.session && req.session.user && req.session.user.privilege >= 10) {
    logger.info(`管理员 ${req.session.user.username} 通过管理员权限检查`);
    return next();
  } else {
    logger.warn(`非管理员用户尝试访问管理员路由: ${req.path}`);
    
    if (req.xhr || req.headers.accept.indexOf('json') > -1) {
      return res.status(403).json({ 
        success: false, 
        message: '需要管理员权限' 
      });
    } else {
      return res.status(403).render('error', {
        title: '权限不足',
        message: '您没有权限访问此页面',
        user: req.session.user
      });
    }
  }
}

// 可选认证 - 如果用户已登录则设置用户信息，否则继续
function optionalAuth(req, res, next) {
  // 设置本地变量供模板使用
  res.locals.user = req.session.user || null;
  next();
}

// 防止已登录用户访问登录/注册页面
function redirectIfAuthenticated(req, res, next) {
  if (req.session && req.session.user) {
    logger.info(`已登录用户 ${req.session.user.username} 尝试访问登录页面，重定向到主页`);
    return res.redirect('/');
  }
  next();
}

module.exports = {
  requireAuth,
  requireAdmin,
  optionalAuth,
  redirectIfAuthenticated
};
