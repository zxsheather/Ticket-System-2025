# Ticket System 2025 Web Frontend 部署指南

## 概述

本文档详细说明了火车票系统Web前端的安装、配置和部署流程，支持开发环境和生产环境的部署。

## 系统要求

### 最低要求
- **Node.js**: >= 16.0.0
- **NPM**: >= 8.0.0
- **内存**: >= 512MB
- **磁盘空间**: >= 100MB

### 推荐配置
- **Node.js**: >= 18.0.0
- **内存**: >= 2GB
- **CPU**: >= 2核心
- **磁盘空间**: >= 1GB

### 依赖服务
- C++后端服务（火车票核心业务逻辑）
- Redis（可选，用于session存储）
- SMTP邮件服务（可选，用于通知）

## 快速开始

### 1. 获取源代码

```bash
# 克隆项目
git clone https://github.com/username/Ticket-System-2025.git
cd Ticket-System-2025/web-frontend

# 或者直接下载并解压源代码包
```

### 2. 安装依赖

```bash
# 安装Node.js依赖
npm install

# 如果需要，安装全局工具
npm install -g pm2 nodemon
```

### 3. 配置环境

```bash
# 复制环境配置文件
cp .env.example .env

# 编辑配置文件
nano .env
```

### 4. 启动应用

```bash
# 开发环境
npm run dev
# 或使用启动脚本
./start-enhanced.sh dev

# 生产环境
npm start
# 或使用启动脚本
./start-enhanced.sh prod
```

## 详细安装指南

### 环境准备

#### Ubuntu/Debian 系统

```bash
# 更新系统包
sudo apt update && sudo apt upgrade -y

# 安装Node.js和NPM
curl -fsSL https://deb.nodesource.com/setup_18.x | sudo -E bash -
sudo apt-get install -y nodejs

# 验证安装
node --version
npm --version

# 安装PM2
sudo npm install -g pm2

# 安装其他工具
sudo apt install -y git curl wget nano
```

#### CentOS/RHEL 系统

```bash
# 安装Node.js
sudo dnf module install nodejs:18/common

# 或使用NodeSource仓库
curl -fsSL https://rpm.nodesource.com/setup_18.x | sudo bash -
sudo dnf install -y nodejs npm

# 安装PM2
sudo npm install -g pm2

# 启用PM2开机自启
pm2 startup
```

#### macOS 系统

```bash
# 使用Homebrew安装
brew install node npm

# 安装PM2
npm install -g pm2

# 验证安装
node --version
npm --version
```

### 项目安装

#### 1. 下载项目

```bash
# 方式1：Git克隆
git clone https://github.com/username/Ticket-System-2025.git
cd Ticket-System-2025/web-frontend

# 方式2：下载压缩包
wget https://github.com/username/Ticket-System-2025/archive/main.zip
unzip main.zip
cd Ticket-System-2025-main/web-frontend
```

#### 2. 安装依赖

```bash
# 清理npm缓存（可选）
npm cache clean --force

# 安装项目依赖
npm install

# 如果遇到权限问题（Linux/macOS）
sudo npm install --unsafe-perm=true --allow-root

# 安装开发依赖（开发环境）
npm install --dev
```

#### 3. 权限设置

```bash
# 设置启动脚本权限
chmod +x start-enhanced.sh

# 创建日志目录
mkdir -p logs
chmod 755 logs

# 设置文件权限
find . -type f -name "*.js" -exec chmod 644 {} \;
find . -type f -name "*.json" -exec chmod 644 {} \;
```

## 配置指南

### 环境变量配置

```bash
# 复制配置模板
cp .env.example .env

# 编辑配置文件
nano .env
```

#### 关键配置项说明

```env
# 基本配置
NODE_ENV=production          # 运行环境
PORT=3000                   # 服务端口
HOST=0.0.0.0               # 监听地址

# 安全配置
SESSION_SECRET=your-secret-key-here  # 会话密钥（必须修改）

# 后端连接
BACKEND_HOST=localhost      # C++后端地址
BACKEND_PORT=12345         # C++后端端口

# 日志级别
LOG_LEVEL=info             # 日志级别
```

### SSL/HTTPS 配置（生产环境推荐）

```env
# 启用HTTPS
ENABLE_HTTPS=true
SSL_CERT_PATH=/path/to/certificate.pem
SSL_KEY_PATH=/path/to/private-key.pem
```

生成自签名证书（测试用）：

```bash
# 创建SSL目录
mkdir -p ssl

# 生成私钥和证书
openssl req -x509 -newkey rsa:4096 -keyout ssl/key.pem -out ssl/cert.pem -days 365 -nodes

# 设置权限
chmod 600 ssl/key.pem
chmod 644 ssl/cert.pem
```

### 反向代理配置

#### Nginx 配置示例

```nginx
server {
    listen 80;
    server_name your-domain.com;
    
    # HTTP重定向到HTTPS
    return 301 https://$server_name$request_uri;
}

server {
    listen 443 ssl http2;
    server_name your-domain.com;
    
    # SSL配置
    ssl_certificate /path/to/cert.pem;
    ssl_certificate_key /path/to/key.pem;
    
    # 安全头
    add_header X-Frame-Options DENY;
    add_header X-Content-Type-Options nosniff;
    add_header X-XSS-Protection "1; mode=block";
    
    # 代理到Node.js应用
    location / {
        proxy_pass http://localhost:3000;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection 'upgrade';
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
        proxy_cache_bypass $http_upgrade;
    }
    
    # WebSocket支持
    location /socket.io/ {
        proxy_pass http://localhost:3000;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection "upgrade";
        proxy_set_header Host $host;
    }
    
    # 静态文件缓存
    location ~* \.(css|js|png|jpg|jpeg|gif|ico|svg)$ {
        expires 1y;
        add_header Cache-Control "public, immutable";
    }
}
```

#### Apache 配置示例

```apache
<VirtualHost *:80>
    ServerName your-domain.com
    Redirect permanent / https://your-domain.com/
</VirtualHost>

<VirtualHost *:443>
    ServerName your-domain.com
    
    # SSL配置
    SSLEngine on
    SSLCertificateFile /path/to/cert.pem
    SSLCertificateKeyFile /path/to/key.pem
    
    # 反向代理
    ProxyPreserveHost On
    ProxyPass / http://localhost:3000/
    ProxyPassReverse / http://localhost:3000/
    
    # WebSocket支持
    RewriteEngine On
    RewriteCond %{REQUEST_URI} ^/socket.io [NC]
    RewriteCond %{QUERY_STRING} transport=websocket [NC]
    RewriteRule /(.*) ws://localhost:3000/$1 [P,L]
    
    # 静态文件缓存
    <LocationMatch "\.(css|js|png|jpg|jpeg|gif|ico|svg)$">
        ExpiresActive On
        ExpiresDefault "access plus 1 year"
        Header append Cache-Control "public"
    </LocationMatch>
</VirtualHost>
```

## 部署方式

### 1. 开发环境部署

```bash
# 使用nodemon自动重启
npm run dev

# 或使用启动脚本
./start-enhanced.sh dev

# 直接启动
node app.js
```

### 2. 生产环境部署

#### 方式1：使用PM2（推荐）

```bash
# 启动应用
./start-enhanced.sh prod

# 或手动使用PM2
pm2 start ecosystem.config.js --env production

# 查看状态
pm2 status
pm2 logs ticket-web-frontend

# 设置开机自启
pm2 startup
pm2 save
```

#### 方式2：使用系统服务

创建systemd服务文件：

```bash
sudo nano /etc/systemd/system/ticket-web-frontend.service
```

```ini
[Unit]
Description=Ticket System 2025 Web Frontend
After=network.target

[Service]
Type=simple
User=node
Group=node
WorkingDirectory=/path/to/web-frontend
Environment=NODE_ENV=production
Environment=PORT=3000
ExecStart=/usr/bin/node app.js
Restart=always
RestartSec=10
StandardOutput=syslog
StandardError=syslog
SyslogIdentifier=ticket-web-frontend

[Install]
WantedBy=multi-user.target
```

启动服务：

```bash
# 重载systemd配置
sudo systemctl daemon-reload

# 启动服务
sudo systemctl start ticket-web-frontend

# 开机自启
sudo systemctl enable ticket-web-frontend

# 查看状态
sudo systemctl status ticket-web-frontend
```

#### 方式3：使用Docker

创建Dockerfile：

```dockerfile
FROM node:18-alpine

# 创建工作目录
WORKDIR /app

# 复制package文件
COPY package*.json ./

# 安装依赖
RUN npm ci --only=production

# 复制应用代码
COPY . .

# 创建非root用户
RUN addgroup -g 1001 -S nodejs
RUN adduser -S nextjs -u 1001

# 设置权限
RUN chown -R nextjs:nodejs /app
USER nextjs

# 暴露端口
EXPOSE 3000

# 启动应用
CMD ["node", "app.js"]
```

构建和运行：

```bash
# 构建镜像
docker build -t ticket-web-frontend .

# 运行容器
docker run -d \
  --name ticket-web-frontend \
  -p 3000:3000 \
  -e NODE_ENV=production \
  -e SESSION_SECRET=your-secret-key \
  --restart unless-stopped \
  ticket-web-frontend
```

使用docker-compose：

```yaml
version: '3.8'

services:
  web-frontend:
    build: .
    ports:
      - "3000:3000"
    environment:
      - NODE_ENV=production
      - SESSION_SECRET=your-secret-key
      - BACKEND_HOST=backend
      - BACKEND_PORT=12345
    depends_on:
      - backend
      - redis
    restart: unless-stopped
    volumes:
      - ./logs:/app/logs

  backend:
    image: ticket-system-backend
    ports:
      - "12345:12345"
    volumes:
      - ./data:/app/data

  redis:
    image: redis:alpine
    ports:
      - "6379:6379"
    restart: unless-stopped

  nginx:
    image: nginx:alpine
    ports:
      - "80:80"
      - "443:443"
    volumes:
      - ./nginx.conf:/etc/nginx/nginx.conf
      - ./ssl:/etc/nginx/ssl
    depends_on:
      - web-frontend
    restart: unless-stopped
```

## 运维管理

### 健康检查

```bash
# 应用健康检查
curl http://localhost:3000/health

# 系统状态检查
curl http://localhost:3000/api/system/status
```

### 日志管理

```bash
# 查看应用日志
tail -f logs/application.log

# 查看错误日志
tail -f logs/error.log

# 查看PM2日志
pm2 logs ticket-web-frontend

# 日志轮转（使用logrotate）
sudo nano /etc/logrotate.d/ticket-web-frontend
```

### 性能监控

```bash
# PM2监控
pm2 monit

# 系统资源监控
htop
iostat
netstat -tulpn
```

### 备份和恢复

```bash
# 创建备份脚本
#!/bin/bash
BACKUP_DIR="/backup/ticket-web-frontend"
DATE=$(date +%Y%m%d_%H%M%S)

# 创建备份目录
mkdir -p $BACKUP_DIR

# 备份配置文件
tar -czf $BACKUP_DIR/config_$DATE.tar.gz .env ecosystem.config.js

# 备份日志文件
tar -czf $BACKUP_DIR/logs_$DATE.tar.gz logs/

# 清理旧备份（保留7天）
find $BACKUP_DIR -name "*.tar.gz" -mtime +7 -delete
```

### 更新部署

```bash
# 备份当前版本
cp -r /path/to/web-frontend /path/to/web-frontend.backup

# 拉取最新代码
git pull origin main

# 安装新依赖
npm install

# 重启应用
pm2 restart ticket-web-frontend

# 或使用脚本
./start-enhanced.sh restart
```

## 故障排除

### 常见问题

#### 1. 端口占用

```bash
# 查找占用端口的进程
sudo netstat -tulpn | grep :3000
sudo lsof -i :3000

# 终止进程
sudo kill -9 <PID>
```

#### 2. 权限问题

```bash
# 修复文件权限
sudo chown -R $USER:$USER /path/to/web-frontend
chmod -R 755 /path/to/web-frontend
```

#### 3. 依赖安装失败

```bash
# 清理npm缓存
npm cache clean --force

# 删除node_modules重新安装
rm -rf node_modules package-lock.json
npm install
```

#### 4. 内存不足

```bash
# 检查内存使用
free -h
ps aux --sort=-%mem | head

# 增加Node.js内存限制
node --max-old-space-size=2048 app.js
```

### 调试模式

```bash
# 启用调试模式
DEBUG=ticket:* node app.js

# 使用Node.js检查器
node --inspect app.js

# 生产环境调试
NODE_ENV=production DEBUG=ticket:error node app.js
```

## 安全建议

### 基本安全措施

1. **更改默认密钥**
   ```env
   SESSION_SECRET=your-unique-secret-key-here
   ```

2. **启用HTTPS**
   ```env
   ENABLE_HTTPS=true
   SSL_CERT_PATH=/path/to/cert.pem
   SSL_KEY_PATH=/path/to/key.pem
   ```

3. **配置防火墙**
   ```bash
   # Ubuntu/Debian
   sudo ufw allow 22
   sudo ufw allow 80
   sudo ufw allow 443
   sudo ufw enable
   
   # CentOS/RHEL
   sudo firewall-cmd --permanent --add-service=ssh
   sudo firewall-cmd --permanent --add-service=http
   sudo firewall-cmd --permanent --add-service=https
   sudo firewall-cmd --reload
   ```

4. **定期更新依赖**
   ```bash
   npm audit
   npm audit fix
   npm update
   ```

5. **限制文件权限**
   ```bash
   chmod 600 .env
   chmod 600 ssl/*.pem
   chmod 755 logs/
   ```

### 生产环境安全清单

- [ ] 更改所有默认密钥和密码
- [ ] 启用HTTPS加密
- [ ] 配置防火墙规则
- [ ] 禁用不必要的服务
- [ ] 设置日志监控和告警
- [ ] 配置自动备份
- [ ] 启用访问日志
- [ ] 设置速率限制
- [ ] 配置安全头
- [ ] 定期安全更新

## 技术支持

### 获取帮助

- **项目文档**: [GitHub Wiki](https://github.com/username/Ticket-System-2025/wiki)
- **问题报告**: [GitHub Issues](https://github.com/username/Ticket-System-2025/issues)
- **讨论区**: [GitHub Discussions](https://github.com/username/Ticket-System-2025/discussions)

### 版本信息

- **当前版本**: 1.0.0
- **Node.js版本**: >= 16.0.0
- **最后更新**: 2024年12月

---

*本部署指南将随着项目更新持续维护和完善。*
