// 错误处理中间件
const winston = require('winston');

const logger = winston.createLogger({
  level: 'error',
  format: winston.format.combine(
    winston.format.timestamp(),
    winston.format.errors({ stack: true }),
    winston.format.json()
  ),
  defaultMeta: { service: 'error-handler' },
  transports: [
    new winston.transports.File({ filename: 'logs/error.log' }),
    new winston.transports.Console({
      format: winston.format.simple()
    })
  ]
});

// 404错误处理
function notFoundHandler(req, res, next) {
  const error = new Error(`未找到页面: ${req.originalUrl}`);
  error.status = 404;
  next(error);
}

// 通用错误处理
function errorHandler(err, req, res, next) {
  // 设置默认状态码
  err.status = err.status || 500;
  
  // 记录错误日志
  logger.error({
    message: err.message,
    stack: err.stack,
    url: req.originalUrl,
    method: req.method,
    ip: req.ip,
    userAgent: req.get('User-Agent'),
    user: req.session?.user?.username || 'anonymous'
  });
  
  // 开发环境显示详细错误信息
  const isDevelopment = process.env.NODE_ENV !== 'production';
  
  // AJAX请求返回JSON格式错误
  if (req.xhr || req.headers.accept.indexOf('json') > -1) {
    return res.status(err.status).json({
      success: false,
      message: err.message,
      ...(isDevelopment && { stack: err.stack })
    });
  }
  
  // 普通请求渲染错误页面
  res.status(err.status).render('error', {
    title: getErrorTitle(err.status),
    message: err.message,
    status: err.status,
    user: req.session?.user || null,
    ...(isDevelopment && { stack: err.stack })
  });
}

// 获取错误标题
function getErrorTitle(status) {
  switch (status) {
    case 400:
      return '请求错误';
    case 401:
      return '未授权';
    case 403:
      return '禁止访问';
    case 404:
      return '页面未找到';
    case 500:
      return '服务器内部错误';
    default:
      return '发生错误';
  }
}

// 验证错误处理
function validationErrorHandler(errors) {
  const errorMessages = errors.map(error => error.msg);
  const err = new Error(errorMessages.join(', '));
  err.status = 400;
  return err;
}

module.exports = {
  notFoundHandler,
  errorHandler,
  validationErrorHandler
};
