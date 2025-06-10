/**
 * PM2 Ecosystem Configuration for Ticket System 2025 Web Frontend
 * 生产环境部署配置文件
 */

module.exports = {
  apps: [{
    name: 'ticket-web-frontend',
    script: 'app.js',
    
    // 实例配置
    instances: process.env.PM2_INSTANCES || 'max', // 使用所有CPU核心
    exec_mode: 'cluster', // 集群模式
    
    // 环境配置
    env: {
      NODE_ENV: 'development',
      PORT: 3000,
      LOG_LEVEL: 'info'
    },
    env_production: {
      NODE_ENV: 'production',
      PORT: process.env.PORT || 3000,
      LOG_LEVEL: process.env.LOG_LEVEL || 'warn'
    },
    
    // 日志配置
    error_file: './logs/pm2-error.log',
    out_file: './logs/pm2-out.log',
    log_file: './logs/pm2-combined.log',
    time: true,
    log_date_format: 'YYYY-MM-DD HH:mm:ss Z',
    
    // 性能配置
    max_memory_restart: '1G', // 内存超过1GB时重启
    node_args: '--max-old-space-size=1024', // Node.js最大内存使用
    
    // 自动重启配置
    autorestart: true,
    watch: false, // 生产环境不监听文件变化
    max_restarts: 10, // 最大重启次数
    min_uptime: '10s', // 最小运行时间
    
    // 健康检查
    health_check_grace_period: 3000,
    
    // 进程管理
    kill_timeout: 5000, // 强制杀死进程的超时时间
    listen_timeout: 3000, // 监听端口的超时时间
    
    // 自定义配置
    merge_logs: true, // 合并日志
    
    // 监控配置
    pmx: true,
    
    // 进程名称
    instance_var: 'INSTANCE_ID',
    
    // 其他配置
    vizion: false, // 禁用版本控制功能
    
    // 环境变量
    env_file: './.env' // 从.env文件加载环境变量
  }],
  
  // 部署配置 (可选)
  deploy: {
    production: {
      user: 'node',
      host: 'your-production-server.com',
      ref: 'origin/main',
      repo: 'git@github.com:username/ticket-system-2025.git',
      path: '/var/www/production',
      'pre-deploy-local': '',
      'post-deploy': 'npm install && pm2 reload ecosystem.config.js --env production',
      'pre-setup': ''
    }
  }
};
