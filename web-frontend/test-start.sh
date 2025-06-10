#!/bin/bash

# 火车票系统Web前端测试启动脚本
# 用于快速启动和测试Web前端服务

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 打印带颜色的消息
print_message() {
    local color=$1
    local message=$2
    echo -e "${color}[$(date +'%Y-%m-%d %H:%M:%S')] ${message}${NC}"
}

print_success() {
    print_message "$GREEN" "✓ $1"
}

print_error() {
    print_message "$RED" "✗ $1"
}

print_warning() {
    print_message "$YELLOW" "⚠ $1"
}

print_info() {
    print_message "$BLUE" "ℹ $1"
}

# 检查Node.js版本
check_node() {
    if ! command -v node &> /dev/null; then
        print_error "Node.js 未安装"
        exit 1
    fi
    
    local node_version=$(node --version | cut -d'v' -f2)
    local major_version=$(echo $node_version | cut -d'.' -f1)
    
    if [ "$major_version" -lt 14 ]; then
        print_error "Node.js 版本过低 (当前: $node_version, 需要: >= 14.0.0)"
        exit 1
    fi
    
    print_success "Node.js 版本检查通过: $node_version"
}

# 检查依赖
check_dependencies() {
    if [ ! -d "node_modules" ]; then
        print_warning "依赖未安装，正在安装..."
        npm install
    else
        print_success "依赖检查通过"
    fi
}

# 创建必要的目录
create_directories() {
    local dirs=("logs" "public/uploads" "tmp")
    
    for dir in "${dirs[@]}"; do
        if [ ! -d "$dir" ]; then
            mkdir -p "$dir"
            print_success "创建目录: $dir"
        fi
    done
}

# 检查环境配置
check_environment() {
    if [ ! -f ".env" ]; then
        if [ -f ".env.example" ]; then
            cp .env.example .env
            print_warning "从 .env.example 复制环境配置"
        else
            print_error "环境配置文件不存在"
            exit 1
        fi
    else
        print_success "环境配置检查通过"
    fi
}

# 检查C++后端
check_backend() {
    local backend_path="../build/code"
    if [ ! -f "$backend_path" ]; then
        print_warning "C++后端程序不存在: $backend_path"
        print_info "Web前端将以独立模式运行（部分功能可能不可用）"
    else
        print_success "C++后端程序检查通过"
    fi
}

# 启动服务
start_service() {
    print_info "启动火车票系统Web前端服务..."
    print_info "访问地址: http://localhost:3000"
    print_info "按 Ctrl+C 停止服务"
    echo
    
    # 设置环境变量
    export NODE_ENV=development
    
    # 启动服务
    node app.js
}

# 主函数
main() {
    print_info "火车票系统Web前端 - 测试启动脚本"
    print_info "========================================"
    
    # 检查当前目录
    if [ ! -f "app.js" ]; then
        print_error "请在Web前端项目根目录下运行此脚本"
        exit 1
    fi
    
    # 执行检查
    check_node
    check_dependencies
    create_directories
    check_environment
    check_backend
    
    print_success "所有检查完成，准备启动服务"
    echo
    
    # 启动服务
    start_service
}

# 捕获中断信号
trap 'print_info "正在停止服务..."; exit 0' INT TERM

# 运行主函数
main "$@"
