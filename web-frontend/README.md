# 火车票系统 Web 前端

这是一个基于 Node.js + Express + EJS 的现代化 Web 前端，为火车票管理系统提供完整的用户界面和管理功能。

## 🚀 功能特性

### 用户功能
- **用户认证**: 注册、登录、个人资料管理
- **票务查询**: 实时查询火车票信息、座位余量
- **在线购票**: 支持多种座位类型的票务购买
- **订单管理**: 查看历史订单、退票操作
- **实时通知**: WebSocket 实时通知系统

### 管理员功能
- **系统监控**: 实时系统性能监控和健康检查
- **用户管理**: 用户账户管理、权限控制
- **列车管理**: 列车信息管理、座位配置
- **订单管理**: 全局订单查看和管理
- **数据统计**: 系统使用统计和报表

### 技术特性
- **响应式设计**: 支持桌面端和移动端访问
- **实时通信**: WebSocket 支持实时数据更新
- **安全防护**: 请求频率限制、CSRF 防护、输入验证
- **性能优化**: 资源压缩、缓存策略、负载均衡
- **日志系统**: 完整的日志记录和错误追踪

## 📋 系统要求

- **Node.js**: >= 14.0.0
- **npm**: >= 6.0.0
- **操作系统**: Linux, macOS, Windows
- **内存**: 至少 512MB 可用内存
- **存储**: 至少 100MB 可用磁盘空间

## 🛠️ 安装和配置

### 1. 克隆项目

```bash
cd /path/to/Ticket-System-2025/web-frontend
```

### 2. 安装依赖

```bash
npm install
```

### 3. 环境配置

复制环境配置文件并根据需要修改：

```bash
cp .env.example .env
```

主要配置项：

```bash
# 基本配置
NODE_ENV=development
PORT=3000
HOST=0.0.0.0

# 会话密钥（生产环境请使用强随机字符串）
SESSION_SECRET=your-super-secret-session-key

# C++后端连接配置
BACKEND_HOST=localhost
BACKEND_PORT=12345
BACKEND_TIMEOUT=30000

# 安全配置
RATE_LIMIT_MAX=100
CORS_ORIGIN=http://localhost:3000
```

### 4. 创建必要目录

```bash
mkdir -p logs public/uploads tmp
```

## 🚀 启动服务

### 开发模式

```bash
# 使用测试启动脚本（推荐）
./test-start.sh

# 或直接启动
npm start

# 或使用开发模式（支持热重载）
npm run dev
```

### 生产模式

```bash
# 使用 PM2 进程管理器
npm install -g pm2
pm2 start ecosystem.config.js

# 或使用增强启动脚本
./start-enhanced.sh
```

## 📱 访问界面

服务启动后，可以通过以下地址访问：

- **用户界面**: http://localhost:3000
- **登录页面**: http://localhost:3000/login
- **注册页面**: http://localhost:3000/register
- **管理员面板**: http://localhost:3000/admin
- **系统健康检查**: http://localhost:3000/health

## 🔧 API 接口

### 用户 API
- `POST /api/user/login` - 用户登录
- `POST /api/user/register` - 用户注册
- `POST /api/user/logout` - 用户登出
- `GET /api/user/profile` - 获取用户资料
- `PUT /api/user/profile` - 更新用户资料

### 票务 API
- `GET /api/ticket/search` - 查询车票
- `GET /api/ticket/transfer` - 查询中转方案
- `POST /api/order/buy` - 购买车票
- `GET /api/order/list` - 查询订单
- `POST /api/order/refund` - 退票

### 管理员 API
- `GET /api/admin/users` - 用户管理
- `GET /api/admin/trains` - 列车管理
- `GET /api/admin/orders` - 订单管理
- `GET /api/system/status` - 系统状态

## 🏗️ 项目结构

```
web-frontend/
├── app.js                 # 主应用程序入口
├── package.json          # 依赖配置
├── .env                  # 环境配置
├── config/               # 配置文件
│   ├── app.js           # 应用配置
│   └── database.js      # 数据库配置
├── controllers/          # 控制器
│   └── ticketController.js
├── middleware/           # 中间件
│   ├── auth.js          # 认证中间件
│   ├── error.js         # 错误处理
│   └── validation.js    # 输入验证
├── routes/              # 路由定义
│   ├── user.js          # 用户路由
│   ├── ticket.js        # 票务路由
│   ├── order.js         # 订单路由
│   ├── train.js         # 列车路由
│   └── admin.js         # 管理员路由
├── views/               # 视图模板
│   ├── layout.ejs       # 主布局
│   ├── index.ejs        # 首页
│   ├── login.ejs        # 登录页
│   ├── register.ejs     # 注册页
│   ├── search.ejs       # 搜索页
│   ├── orders.ejs       # 订单页
│   ├── profile.ejs      # 个人资料
│   ├── admin.ejs        # 管理面板
│   └── partials/        # 页面组件
├── public/              # 静态资源
│   ├── css/            # 样式文件
│   ├── js/             # 前端脚本
│   └── images/         # 图片资源
├── utils/               # 工具模块
│   ├── logger.js        # 日志系统
│   ├── notificationSystem.js  # 通知系统
│   └── systemMonitor.js # 系统监控
└── logs/               # 日志文件
```

## 🔐 安全特性

- **会话管理**: 安全的会话配置和过期处理
- **输入验证**: 所有用户输入的严格验证
- **CSRF 防护**: 跨站请求伪造防护
- **XSS 防护**: 跨站脚本攻击防护
- **请求限制**: API 调用频率限制
- **权限控制**: 基于角色的访问控制

## 📊 监控和日志

### 系统监控
- CPU 和内存使用率监控
- 磁盘空间使用监控
- 网络连接状态监控
- 应用性能指标监控

### 日志记录
- 访问日志记录
- 错误日志记录
- 系统操作日志
- 用户行为日志

### 实时通知
- WebSocket 实时通知
- 系统状态变更通知
- 用户操作结果通知
- 管理员警报通知

## 🔧 开发指南

### 添加新功能

1. **创建路由**: 在 `routes/` 目录下创建或修改路由文件
2. **添加视图**: 在 `views/` 目录下创建 EJS 模板
3. **编写样式**: 在 `public/css/` 中添加 CSS 样式
4. **前端脚本**: 在 `public/js/` 中添加 JavaScript 功能
5. **测试验证**: 运行测试确保功能正常

### 代码规范

- 使用 ES6+ 语法
- 遵循 Node.js 最佳实践
- 注重代码可读性和维护性
- 完整的错误处理
- 详细的注释说明

## 🚨 故障排除

### 常见问题

1. **端口冲突**
   ```bash
   # 检查端口占用
   lsof -i :3000
   # 修改 .env 中的 PORT 配置
   ```

2. **依赖安装失败**
   ```bash
   # 清除缓存重新安装
   npm cache clean --force
   npm install
   ```

3. **C++后端连接失败**
   - 检查后端程序是否已编译
   - 确认后端程序正在运行
   - 验证端口配置是否正确

4. **权限问题**
   ```bash
   # 确保有足够的文件权限
   chmod +x start-enhanced.sh
   chmod 755 logs/
   ```

### 日志查看

```bash
# 查看应用日志
tail -f logs/combined.log

# 查看错误日志
tail -f logs/error.log

# 查看系统监控日志
tail -f logs/system-monitor.log
```

## 📈 性能优化

- **资源压缩**: 启用 gzip 压缩
- **缓存策略**: 静态资源缓存
- **连接池**: 数据库连接优化
- **负载均衡**: 多进程部署支持

## 🤝 贡献指南

1. Fork 项目
2. 创建功能分支
3. 提交更改
4. 推送到分支
5. 创建 Pull Request

## 📄 许可证

MIT License - 详见 LICENSE 文件

## 📞 支持

如有问题或建议，请通过以下方式联系：

- 提交 Issue
- 发送邮件
- 查看文档

---

**火车票系统团队**  
版本: 1.0.0  
更新日期: 2025年5月31日
