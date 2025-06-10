// 应用程序配置文件
// 集中管理所有应用配置

const path = require('path');

const config = {
  // 应用基本信息
  app: {
    name: 'Ticket System 2025 Web Frontend',
    version: '1.0.0',
    description: 'Web GUI frontend for railway ticket booking system',
    author: 'Ticket System Team'
  },
  
  // 服务器配置
  server: {
    port: parseInt(process.env.PORT) || 3000,
    host: process.env.HOST || 'localhost',
    env: process.env.NODE_ENV || 'development'
  },
  
  // 会话配置
  session: {
    secret: process.env.SESSION_SECRET || 'ticket-system-secret-key-2025',
    maxAge: parseInt(process.env.SESSION_MAX_AGE) || 24 * 60 * 60 * 1000, // 24小时
    secure: process.env.NODE_ENV === 'production', // 生产环境启用HTTPS
    sameSite: 'lax'
  },
  
  // 安全配置
  security: {
    enableCSP: process.env.NODE_ENV === 'production',
    enableHSTS: process.env.NODE_ENV === 'production',
    rateLimit: {
      windowMs: 15 * 60 * 1000, // 15分钟
      max: parseInt(process.env.RATE_LIMIT_MAX) || 100,
      skipSuccessfulRequests: true
    },
    cors: {
      origin: process.env.CORS_ORIGIN ? process.env.CORS_ORIGIN.split(',') : true,
      credentials: true,
      methods: ['GET', 'POST', 'PUT', 'DELETE', 'OPTIONS'],
      allowedHeaders: ['Content-Type', 'Authorization', 'X-Requested-With']
    }
  },
  
  // 日志配置
  logging: {
    level: process.env.LOG_LEVEL || 'info',
    format: 'json',
    enableConsole: process.env.LOG_CONSOLE !== 'false',
    enableFile: process.env.LOG_FILE !== 'false',
    maxFiles: parseInt(process.env.LOG_MAX_FILES) || 7,
    maxSize: process.env.LOG_MAX_SIZE || '20m',
    directory: path.join(__dirname, '../logs')
  },
  
  // Socket.IO配置
  socketio: {
    cors: {
      origin: process.env.CORS_ORIGIN || "*",
      methods: ["GET", "POST"],
      credentials: true
    },
    transports: ['websocket', 'polling'],
    pingTimeout: 60000,
    pingInterval: 25000
  },
  
  // 文件上传配置
  upload: {
    maxFileSize: parseInt(process.env.UPLOAD_MAX_SIZE) || 5 * 1024 * 1024, // 5MB
    allowedTypes: ['image/jpeg', 'image/png', 'image/gif', 'application/pdf'],
    directory: path.join(__dirname, '../public/uploads')
  },
  
  // 系统监控配置
  monitoring: {
    enabled: process.env.MONITORING_ENABLED !== 'false',
    interval: parseInt(process.env.MONITORING_INTERVAL) || 30000, // 30秒
    alertThresholds: {
      cpu: parseInt(process.env.ALERT_CPU_THRESHOLD) || 80,
      memory: parseInt(process.env.ALERT_MEMORY_THRESHOLD) || 80,
      disk: parseInt(process.env.ALERT_DISK_THRESHOLD) || 90
    }
  },
  
  // 通知系统配置
  notifications: {
    enabled: process.env.NOTIFICATIONS_ENABLED !== 'false',
    maxNotifications: parseInt(process.env.MAX_NOTIFICATIONS) || 100,
    defaultExpiry: parseInt(process.env.NOTIFICATION_EXPIRY) || 7 * 24 * 60 * 60 * 1000, // 7天
    types: {
      system: { priority: 'high', icon: 'fas fa-cog' },
      order: { priority: 'medium', icon: 'fas fa-ticket-alt' },
      user: { priority: 'low', icon: 'fas fa-user' },
      maintenance: { priority: 'high', icon: 'fas fa-tools' }
    }
  },
  
  // 缓存配置
  cache: {
    enabled: process.env.CACHE_ENABLED !== 'false',
    ttl: parseInt(process.env.CACHE_TTL) || 300, // 5分钟
    maxKeys: parseInt(process.env.CACHE_MAX_KEYS) || 1000
  },
  
  // API配置
  api: {
    prefix: '/api',
    version: 'v1',
    timeout: parseInt(process.env.API_TIMEOUT) || 30000,
    maxRetries: parseInt(process.env.API_MAX_RETRIES) || 3
  },
  
  // 前端资源配置
  assets: {
    enableCompression: process.env.NODE_ENV === 'production',
    enableMinification: process.env.NODE_ENV === 'production',
    staticMaxAge: process.env.NODE_ENV === 'production' ? '1d' : 0,
    cacheControl: process.env.NODE_ENV === 'production'
  },
  
  // 开发模式配置
  development: {
    enableHotReload: process.env.HOT_RELOAD === 'true',
    enableDebugMode: process.env.DEBUG_MODE === 'true',
    showErrorDetails: true,
    enableMockData: process.env.MOCK_DATA === 'true'
  }
};

// 环境特定的配置覆盖
if (config.server.env === 'production') {
  // 生产环境优化
  config.logging.level = 'warn';
  config.security.enableCSP = true;
  config.security.enableHSTS = true;
  config.session.secure = true;
} else if (config.server.env === 'test') {
  // 测试环境配置
  config.logging.level = 'error';
  config.logging.enableConsole = false;
  config.monitoring.enabled = false;
  config.notifications.enabled = false;
}

module.exports = config;
