#!/bin/bash

# Ticket System 2025 Web Frontend 启动脚本
# 支持开发环境和生产环境的启动配置

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# 默认配置
NODE_ENV=${NODE_ENV:-development}
PORT=${PORT:-3000}
LOG_LEVEL=${LOG_LEVEL:-info}

# 函数定义
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 检查Node.js版本
check_node_version() {
    if ! command -v node &> /dev/null; then
        print_error "Node.js 未安装，请先安装 Node.js"
        exit 1
    fi
    
    NODE_VERSION=$(node --version | cut -d'v' -f2)
    REQUIRED_VERSION="16.0.0"
    
    if ! node -e "process.exit(require('semver').gte('$NODE_VERSION', '$REQUIRED_VERSION') ? 0 : 1)" 2>/dev/null; then
        print_warning "Node.js 版本 $NODE_VERSION 可能不兼容，建议使用 $REQUIRED_VERSION 或更高版本"
    else
        print_success "Node.js 版本检查通过: $NODE_VERSION"
    fi
}

# 检查依赖
check_dependencies() {
    print_info "检查依赖包..."
    
    if [ ! -d "node_modules" ]; then
        print_warning "node_modules 目录不存在，正在安装依赖..."
        npm install
    else
        # 检查package.json是否有更新
        if [ "package.json" -nt "node_modules" ]; then
            print_warning "package.json 已更新，正在重新安装依赖..."
            npm install
        fi
    fi
    
    print_success "依赖检查完成"
}

# 检查日志目录
check_log_directory() {
    if [ ! -d "logs" ]; then
        print_info "创建日志目录..."
        mkdir -p logs
        
        # 创建初始日志文件
        touch logs/application.log
        touch logs/error.log
        touch logs/auth.log
        touch logs/access.log
        
        print_success "日志目录创建完成"
    fi
}

# 检查配置文件
check_config() {
    print_info "检查配置文件..."
    
    # 检查环境变量文件
    if [ ! -f ".env" ] && [ "$NODE_ENV" = "production" ]; then
        print_warning "生产环境缺少 .env 配置文件"
        
        # 创建示例配置文件
        cat > .env.example << EOF
# 火车票系统Web前端环境配置

# 基本配置
NODE_ENV=production
PORT=3000
HOST=0.0.0.0

# 会话配置
SESSION_SECRET=your-super-secret-session-key-here
SESSION_MAX_AGE=86400000

# C++后端连接配置
BACKEND_HOST=localhost
BACKEND_PORT=12345
BACKEND_TIMEOUT=30000

# 安全配置
ENABLE_HTTPS=false
SSL_CERT_PATH=/path/to/cert.pem
SSL_KEY_PATH=/path/to/key.pem

# 日志配置
LOG_LEVEL=info
LOG_MAX_SIZE=10m
LOG_MAX_FILES=5

# 限流配置
RATE_LIMIT_WINDOW_MS=900000
RATE_LIMIT_MAX_REQUESTS=100

# Redis配置 (可选，用于session存储)
REDIS_HOST=localhost
REDIS_PORT=6379
REDIS_PASSWORD=

# 邮件配置 (可选，用于通知)
SMTP_HOST=
SMTP_PORT=587
SMTP_USER=
SMTP_PASS=
EOF
        print_info "已创建 .env.example 配置示例文件"
    fi
}

# 预处理检查
pre_checks() {
    print_info "正在进行启动前检查..."
    
    check_node_version
    check_dependencies
    check_log_directory
    check_config
    
    print_success "启动前检查完成"
}

# 启动开发环境
start_development() {
    print_info "启动开发环境..."
    
    export NODE_ENV=development
    export PORT=${PORT}
    export LOG_LEVEL=${LOG_LEVEL}
    
    # 使用nodemon进行开发，支持自动重启
    if command -v nodemon &> /dev/null; then
        print_info "使用 nodemon 启动开发服务器..."
        nodemon --inspect app.js
    else
        print_warning "nodemon 未安装，使用普通 node 启动"
        print_info "建议安装 nodemon: npm install -g nodemon"
        node app.js
    fi
}

# 启动生产环境
start_production() {
    print_info "启动生产环境..."
    
    export NODE_ENV=production
    export PORT=${PORT}
    export LOG_LEVEL=${LOG_LEVEL}
    
    # 使用PM2进行生产环境部署
    if command -v pm2 &> /dev/null; then
        print_info "使用 PM2 启动生产服务器..."
        
        # 创建PM2配置文件
        cat > ecosystem.config.js << EOF
module.exports = {
  apps: [{
    name: 'ticket-web-frontend',
    script: 'app.js',
    instances: 'max',
    exec_mode: 'cluster',
    env: {
      NODE_ENV: 'production',
      PORT: ${PORT}
    },
    error_file: './logs/pm2-error.log',
    out_file: './logs/pm2-out.log',
    log_file: './logs/pm2-combined.log',
    time: true,
    max_memory_restart: '1G',
    node_args: '--max-old-space-size=1024'
  }]
};
EOF
        
        # 启动或重启PM2应用
        pm2 start ecosystem.config.js
        pm2 save
        
        print_success "生产服务器已启动"
        print_info "使用 'pm2 logs ticket-web-frontend' 查看日志"
        print_info "使用 'pm2 stop ticket-web-frontend' 停止服务"
        
    else
        print_warning "PM2 未安装，使用普通 node 启动"
        print_info "建议安装 PM2: npm install -g pm2"
        node app.js
    fi
}

# 停止服务
stop_service() {
    print_info "停止服务..."
    
    if command -v pm2 &> /dev/null; then
        pm2 stop ticket-web-frontend 2>/dev/null || true
        pm2 delete ticket-web-frontend 2>/dev/null || true
        print_success "PM2 服务已停止"
    fi
    
    # 查找并终止Node.js进程
    PIDS=$(pgrep -f "node.*app.js" || true)
    if [ -n "$PIDS" ]; then
        echo "$PIDS" | xargs kill -TERM
        print_success "Node.js 进程已终止"
    fi
}

# 重启服务
restart_service() {
    print_info "重启服务..."
    stop_service
    sleep 2
    
    if [ "$NODE_ENV" = "production" ]; then
        start_production
    else
        start_development
    fi
}

# 查看状态
show_status() {
    print_info "服务状态:"
    
    if command -v pm2 &> /dev/null; then
        pm2 list
    fi
    
    # 检查端口占用
    if command -v netstat &> /dev/null; then
        print_info "端口占用情况:"
        netstat -tlnp | grep ":$PORT " || print_info "端口 $PORT 未被占用"
    elif command -v ss &> /dev/null; then
        print_info "端口占用情况:"
        ss -tlnp | grep ":$PORT " || print_info "端口 $PORT 未被占用"
    fi
}

# 显示日志
show_logs() {
    if [ -f "logs/application.log" ]; then
        print_info "应用日志 (最后50行):"
        tail -50 logs/application.log
    fi
    
    if command -v pm2 &> /dev/null; then
        print_info "PM2 日志:"
        pm2 logs ticket-web-frontend --lines 20
    fi
}

# 健康检查
health_check() {
    print_info "执行健康检查..."
    
    local url="http://localhost:$PORT"
    
    if command -v curl &> /dev/null; then
        if curl -f -s "$url" > /dev/null; then
            print_success "服务健康检查通过"
            return 0
        else
            print_error "服务健康检查失败"
            return 1
        fi
    else
        print_warning "curl 未安装，无法执行健康检查"
        return 1
    fi
}

# 显示帮助信息
show_help() {
    echo "Ticket System 2025 Web Frontend 启动脚本"
    echo ""
    echo "用法: $0 [命令] [选项]"
    echo ""
    echo "命令:"
    echo "  start       启动服务 (根据 NODE_ENV 环境变量)"
    echo "  stop        停止服务"
    echo "  restart     重启服务"
    echo "  status      显示服务状态"
    echo "  logs        显示日志"
    echo "  health      健康检查"
    echo "  dev         启动开发环境"
    echo "  prod        启动生产环境"
    echo "  help        显示此帮助信息"
    echo ""
    echo "环境变量:"
    echo "  NODE_ENV    运行环境 (development/production)"
    echo "  PORT        服务端口 (默认: 3000)"
    echo "  LOG_LEVEL   日志级别 (默认: info)"
    echo ""
    echo "示例:"
    echo "  $0 start              # 启动服务"
    echo "  $0 dev                # 启动开发环境"
    echo "  PORT=8080 $0 prod     # 在8080端口启动生产环境"
    echo "  $0 stop               # 停止服务"
    echo "  $0 restart            # 重启服务"
}

# 主函数
main() {
    case "${1:-start}" in
        "start")
            pre_checks
            if [ "$NODE_ENV" = "production" ]; then
                start_production
            else
                start_development
            fi
            ;;
        "dev")
            export NODE_ENV=development
            pre_checks
            start_development
            ;;
        "prod")
            export NODE_ENV=production
            pre_checks
            start_production
            ;;
        "stop")
            stop_service
            ;;
        "restart")
            restart_service
            ;;
        "status")
            show_status
            ;;
        "logs")
            show_logs
            ;;
        "health")
            health_check
            ;;
        "help"|"-h"|"--help")
            show_help
            ;;
        *)
            print_error "未知命令: $1"
            show_help
            exit 1
            ;;
    esac
}

# 信号处理
trap 'print_info "收到中断信号，正在退出..."; exit 130' INT TERM

# 执行主函数
main "$@"
