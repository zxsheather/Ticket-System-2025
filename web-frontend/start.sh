#!/bin/bash

# 火车票管理系统Web前端启动脚本

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 项目路径
PROJECT_DIR="/home/zxsheather/programming/ACMOJ/Ticket-System-2025/web-frontend"
BACKEND_DIR="/home/zxsheather/programming/ACMOJ/Ticket-System-2025/build"
PID_FILE="$PROJECT_DIR/app.pid"
LOG_DIR="$PROJECT_DIR/logs"

# 打印带颜色的消息
print_message() {
    local color=$1
    local message=$2
    echo -e "${color}[$(date '+%Y-%m-%d %H:%M:%S')] ${message}${NC}"
}

# 检查Node.js是否已安装
check_node() {
    if ! command -v node &> /dev/null; then
        print_message $RED "错误: Node.js 未安装"
        print_message $YELLOW "请先安装 Node.js (版本 >= 14.0.0)"
        exit 1
    fi
    
    NODE_VERSION=$(node -v | sed 's/v//')
    print_message $GREEN "Node.js 版本: $NODE_VERSION"
}

# 检查npm是否已安装
check_npm() {
    if ! command -v npm &> /dev/null; then
        print_message $RED "错误: npm 未安装"
        exit 1
    fi
    
    NPM_VERSION=$(npm -v)
    print_message $GREEN "npm 版本: $NPM_VERSION"
}

# 检查C++后端是否存在
check_backend() {
    if [ ! -f "$BACKEND_DIR/code" ]; then
        print_message $RED "错误: C++ 后端程序不存在"
        print_message $YELLOW "请先编译 C++ 后端程序: $BACKEND_DIR/code"
        exit 1
    fi
    
    print_message $GREEN "C++ 后端程序已找到"
}

# 创建必要的目录
create_directories() {
    print_message $BLUE "创建必要的目录..."
    
    mkdir -p "$LOG_DIR"
    mkdir -p "$PROJECT_DIR/public/uploads"
    mkdir -p "$PROJECT_DIR/config"
    
    print_message $GREEN "目录创建完成"
}

# 安装依赖
install_dependencies() {
    print_message $BLUE "检查并安装依赖..."
    
    cd "$PROJECT_DIR"
    
    if [ ! -d "node_modules" ] || [ ! -f "package-lock.json" ]; then
        print_message $YELLOW "安装 npm 依赖..."
        npm install
        
        if [ $? -eq 0 ]; then
            print_message $GREEN "依赖安装成功"
        else
            print_message $RED "依赖安装失败"
            exit 1
        fi
    else
        print_message $GREEN "依赖已安装"
    fi
}

# 检查端口是否被占用
check_port() {
    local port=${1:-3000}
    
    if lsof -Pi :$port -sTCP:LISTEN -t >/dev/null; then
        print_message $YELLOW "警告: 端口 $port 已被占用"
        
        # 尝试停止已运行的进程
        if [ -f "$PID_FILE" ]; then
            local old_pid=$(cat "$PID_FILE")
            if kill -0 "$old_pid" 2>/dev/null; then
                print_message $BLUE "停止旧进程 (PID: $old_pid)..."
                kill -TERM "$old_pid"
                sleep 2
                
                if kill -0 "$old_pid" 2>/dev/null; then
                    print_message $YELLOW "强制停止进程..."
                    kill -KILL "$old_pid"
                fi
            fi
            rm -f "$PID_FILE"
        fi
        
        # 再次检查端口
        if lsof -Pi :$port -sTCP:LISTEN -t >/dev/null; then
            print_message $RED "错误: 无法释放端口 $port"
            exit 1
        fi
    fi
}

# 启动应用
start_app() {
    print_message $BLUE "启动 Web 前端服务..."
    
    cd "$PROJECT_DIR"
    
    # 设置环境变量
    export NODE_ENV=${NODE_ENV:-development}
    export PORT=${PORT:-3000}
    
    # 启动应用
    if [ "$NODE_ENV" = "development" ]; then
        # 开发模式使用 nodemon
        if command -v nodemon &> /dev/null; then
            print_message $GREEN "使用开发模式启动 (nodemon)..."
            nodemon app.js &
        else
            print_message $YELLOW "nodemon 未安装，使用普通模式启动..."
            node app.js &
        fi
    else
        # 生产模式
        print_message $GREEN "使用生产模式启动..."
        node app.js &
    fi
    
    local app_pid=$!
    echo $app_pid > "$PID_FILE"
    
    # 等待启动
    sleep 3
    
    if kill -0 "$app_pid" 2>/dev/null; then
        print_message $GREEN "Web 前端服务启动成功!"
        print_message $GREEN "PID: $app_pid"
        print_message $GREEN "访问地址: http://localhost:$PORT"
        print_message $GREEN "日志目录: $LOG_DIR"
    else
        print_message $RED "Web 前端服务启动失败"
        rm -f "$PID_FILE"
        exit 1
    fi
}

# 显示状态
show_status() {
    if [ -f "$PID_FILE" ]; then
        local pid=$(cat "$PID_FILE")
        if kill -0 "$pid" 2>/dev/null; then
            print_message $GREEN "Web 前端服务正在运行 (PID: $pid)"
            
            # 显示内存使用情况
            local memory=$(ps -o pid,vsz,rss,comm -p "$pid" | tail -n 1)
            print_message $BLUE "内存使用: $memory"
            
            # 显示端口占用
            local port_info=$(lsof -Pi -p "$pid" | grep LISTEN)
            if [ ! -z "$port_info" ]; then
                print_message $BLUE "监听端口: $port_info"
            fi
        else
            print_message $RED "Web 前端服务未运行 (PID文件存在但进程不存在)"
            rm -f "$PID_FILE"
        fi
    else
        print_message $YELLOW "Web 前端服务未运行"
    fi
}

# 停止应用
stop_app() {
    if [ -f "$PID_FILE" ]; then
        local pid=$(cat "$PID_FILE")
        if kill -0 "$pid" 2>/dev/null; then
            print_message $BLUE "停止 Web 前端服务 (PID: $pid)..."
            
            # 优雅停止
            kill -TERM "$pid"
            
            # 等待进程结束
            local count=0
            while kill -0 "$pid" 2>/dev/null && [ $count -lt 10 ]; do
                sleep 1
                count=$((count + 1))
            done
            
            # 强制停止
            if kill -0 "$pid" 2>/dev/null; then
                print_message $YELLOW "强制停止进程..."
                kill -KILL "$pid"
                sleep 1
            fi
            
            if ! kill -0 "$pid" 2>/dev/null; then
                print_message $GREEN "Web 前端服务已停止"
                rm -f "$PID_FILE"
            else
                print_message $RED "无法停止 Web 前端服务"
                exit 1
            fi
        else
            print_message $YELLOW "进程不存在，清理PID文件"
            rm -f "$PID_FILE"
        fi
    else
        print_message $YELLOW "Web 前端服务未运行"
    fi
}

# 重启应用
restart_app() {
    print_message $BLUE "重启 Web 前端服务..."
    stop_app
    sleep 2
    start_app
}

# 查看日志
view_logs() {
    if [ -f "$LOG_DIR/combined.log" ]; then
        print_message $BLUE "显示应用日志..."
        tail -f "$LOG_DIR/combined.log"
    else
        print_message $YELLOW "日志文件不存在: $LOG_DIR/combined.log"
    fi
}

# 清理日志
clean_logs() {
    print_message $BLUE "清理日志文件..."
    
    if [ -d "$LOG_DIR" ]; then
        find "$LOG_DIR" -name "*.log" -type f -delete
        print_message $GREEN "日志文件已清理"
    else
        print_message $YELLOW "日志目录不存在"
    fi
}

# 显示帮助信息
show_help() {
    echo "火车票管理系统 Web 前端管理脚本"
    echo ""
    echo "用法: $0 [命令]"
    echo ""
    echo "命令:"
    echo "  start     启动 Web 前端服务"
    echo "  stop      停止 Web 前端服务"
    echo "  restart   重启 Web 前端服务"
    echo "  status    查看服务状态"
    echo "  logs      查看实时日志"
    echo "  clean     清理日志文件"
    echo "  install   安装依赖"
    echo "  help      显示此帮助信息"
    echo ""
    echo "环境变量:"
    echo "  NODE_ENV  运行环境 (development/production)"
    echo "  PORT      服务端口 (默认: 3000)"
    echo ""
    echo "示例:"
    echo "  $0 start              # 启动服务"
    echo "  NODE_ENV=production $0 start  # 生产模式启动"
    echo "  PORT=8080 $0 start    # 指定端口启动"
}

# 主函数
main() {
    case "${1:-start}" in
        "start")
            check_node
            check_npm
            check_backend
            create_directories
            install_dependencies
            check_port ${PORT:-3000}
            start_app
            ;;
        "stop")
            stop_app
            ;;
        "restart")
            restart_app
            ;;
        "status")
            show_status
            ;;
        "logs")
            view_logs
            ;;
        "clean")
            clean_logs
            ;;
        "install")
            check_node
            check_npm
            create_directories
            install_dependencies
            ;;
        "help"|"-h"|"--help")
            show_help
            ;;
        *)
            print_message $RED "未知命令: $1"
            show_help
            exit 1
            ;;
    esac
}

# 确保脚本从正确的目录运行
if [ ! -f "$PROJECT_DIR/app.js" ]; then
    print_message $RED "错误: 找不到 app.js 文件"
    print_message $YELLOW "请确保脚本在正确的项目目录中运行"
    exit 1
fi

# 运行主函数
main "$@"
