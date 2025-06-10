// 数据库配置文件
// 由于系统使用C++后端，这里主要配置与后端的通信参数

const config = {
  development: {
    backend: {
      host: process.env.BACKEND_HOST || 'localhost',
      port: process.env.BACKEND_PORT || 12345,
      timeout: parseInt(process.env.BACKEND_TIMEOUT) || 30000,
      retryAttempts: parseInt(process.env.BACKEND_RETRY_ATTEMPTS) || 3,
      retryDelay: parseInt(process.env.BACKEND_RETRY_DELAY) || 1000
    },
    logging: {
      level: 'debug',
      enableConsole: true,
      enableFile: true
    }
  },
  
  production: {
    backend: {
      host: process.env.BACKEND_HOST || 'localhost',
      port: process.env.BACKEND_PORT || 12345,
      timeout: parseInt(process.env.BACKEND_TIMEOUT) || 30000,
      retryAttempts: parseInt(process.env.BACKEND_RETRY_ATTEMPTS) || 5,
      retryDelay: parseInt(process.env.BACKEND_RETRY_DELAY) || 2000
    },
    logging: {
      level: 'info',
      enableConsole: false,
      enableFile: true
    }
  },
  
  test: {
    backend: {
      host: process.env.BACKEND_HOST || 'localhost',
      port: process.env.BACKEND_PORT || 12346, // 测试环境使用不同端口
      timeout: parseInt(process.env.BACKEND_TIMEOUT) || 10000,
      retryAttempts: parseInt(process.env.BACKEND_RETRY_ATTEMPTS) || 1,
      retryDelay: parseInt(process.env.BACKEND_RETRY_DELAY) || 500
    },
    logging: {
      level: 'error',
      enableConsole: true,
      enableFile: false
    }
  }
};

const environment = process.env.NODE_ENV || 'development';

module.exports = config[environment];
