#!/bin/bash

# 火车票系统Web前端 - 项目完整性检查脚本

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# 计数器
total_checks=0
passed_checks=0
failed_checks=0

# 打印函数
print_header() {
    echo -e "\n${PURPLE}===== $1 =====${NC}\n"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
    ((passed_checks++))
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
    ((failed_checks++))
}

print_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

print_info() {
    echo -e "${BLUE}ℹ $1${NC}"
}

check_file() {
    local file=$1
    local description=$2
    ((total_checks++))
    
    if [ -f "$file" ]; then
        print_success "$description: $file"
    else
        print_error "$description 缺失: $file"
    fi
}

check_directory() {
    local dir=$1
    local description=$2
    ((total_checks++))
    
    if [ -d "$dir" ]; then
        print_success "$description: $dir"
    else
        print_error "$description 缺失: $dir"
    fi
}

check_executable() {
    local file=$1
    local description=$2
    ((total_checks++))
    
    if [ -x "$file" ]; then
        print_success "$description 可执行: $file"
    else
        print_error "$description 不可执行: $file"
    fi
}

# 主检查函数
main() {
    echo -e "${CYAN}"
    echo "╔══════════════════════════════════════════════════════════════╗"
    echo "║                 火车票系统Web前端 - 项目检查                 ║"
    echo "║                      Project Health Check                    ║"
    echo "╚══════════════════════════════════════════════════════════════╝"
    echo -e "${NC}"
    
    # 检查当前目录
    if [ ! -f "app.js" ]; then
        print_error "请在Web前端项目根目录下运行此脚本"
        exit 1
    fi
    
    print_header "核心文件检查"
    check_file "app.js" "主应用入口"
    check_file "package.json" "包配置文件"
    check_file ".env" "环境配置文件"
    check_file ".env.example" "环境配置示例"
    check_file "README.md" "项目说明文档"
    check_file "DEPLOYMENT.md" "部署说明文档"
    
    print_header "配置文件检查"
    check_file "config/app.js" "应用配置"
    check_file "config/database.js" "数据库配置"
    check_file "ecosystem.config.js" "PM2配置"
    
    print_header "路由文件检查"
    check_file "routes/user.js" "用户路由"
    check_file "routes/ticket.js" "票务路由"
    check_file "routes/order.js" "订单路由"
    check_file "routes/train.js" "列车路由"
    check_file "routes/admin.js" "管理员路由"
    
    print_header "中间件检查"
    check_file "middleware/auth.js" "认证中间件"
    check_file "middleware/error.js" "错误处理中间件"
    check_file "middleware/validation.js" "验证中间件"
    
    print_header "控制器检查"
    check_file "controllers/ticketController.js" "票务控制器"
    
    print_header "工具模块检查"
    check_file "utils/logger.js" "日志工具"
    check_file "utils/notificationSystem.js" "通知系统"
    check_file "utils/systemMonitor.js" "系统监控"
    
    print_header "视图模板检查"
    check_file "views/layout.ejs" "主布局模板"
    check_file "views/index.ejs" "首页模板"
    check_file "views/login.ejs" "登录页模板"
    check_file "views/register.ejs" "注册页模板"
    check_file "views/search.ejs" "搜索页模板"
    check_file "views/orders.ejs" "订单页模板"
    check_file "views/profile.ejs" "个人资料模板"
    check_file "views/admin.ejs" "管理员面板模板"
    check_file "views/error.ejs" "错误页模板"
    
    print_header "页面组件检查"
    check_file "views/partials/navbar.ejs" "导航栏组件"
    check_file "views/partials/footer.ejs" "页脚组件"
    check_file "views/partials/buy-ticket-modal.ejs" "购票弹窗组件"
    
    print_header "静态资源检查"
    check_file "public/css/style.css" "主样式文件"
    check_file "public/js/search.js" "搜索页脚本"
    check_file "public/js/admin.js" "管理员脚本"
    
    print_header "目录结构检查"
    check_directory "logs" "日志目录"
    check_directory "public/uploads" "上传目录"
    check_directory "tmp" "临时目录"
    check_directory "node_modules" "依赖模块目录"
    
    print_header "脚本文件检查"
    check_file "start.sh" "基础启动脚本"
    check_file "start-enhanced.sh" "增强启动脚本"
    check_file "test-start.sh" "测试启动脚本"
    check_executable "start.sh" "基础启动脚本"
    check_executable "start-enhanced.sh" "增强启动脚本"
    check_executable "test-start.sh" "测试启动脚本"
    
    print_header "语法检查"
    ((total_checks++))
    if node -c app.js 2>/dev/null; then
        print_success "app.js 语法检查通过"
    else
        print_error "app.js 语法错误"
    fi
    
    # 检查package.json语法
    ((total_checks++))
    if node -e "JSON.parse(require('fs').readFileSync('package.json', 'utf8'))" 2>/dev/null; then
        print_success "package.json 格式正确"
    else
        print_error "package.json 格式错误"
    fi
    
    print_header "依赖检查"
    ((total_checks++))
    if npm list --depth=0 >/dev/null 2>&1; then
        print_success "npm 依赖完整"
    else
        print_warning "npm 依赖可能不完整，建议运行 npm install"
    fi
    
    print_header "外部依赖检查"
    ((total_checks++))
    if command -v node &> /dev/null; then
        local node_version=$(node --version)
        print_success "Node.js 已安装: $node_version"
    else
        print_error "Node.js 未安装"
    fi
    
    ((total_checks++))
    if command -v npm &> /dev/null; then
        local npm_version=$(npm --version)
        print_success "npm 已安装: $npm_version"
    else
        print_error "npm 未安装"
    fi
    
    # C++后端检查
    print_header "后端程序检查"
    ((total_checks++))
    local backend_path="../build/code"
    if [ -f "$backend_path" ]; then
        print_success "C++后端程序存在: $backend_path"
    else
        print_warning "C++后端程序不存在: $backend_path (可选)"
    fi
    
    # 环境变量检查
    print_header "环境配置检查"
    if [ -f ".env" ]; then
        local env_vars=("NODE_ENV" "PORT" "SESSION_SECRET" "BACKEND_HOST" "BACKEND_PORT")
        for var in "${env_vars[@]}"; do
            ((total_checks++))
            if grep -q "^$var=" .env; then
                print_success "环境变量 $var 已配置"
            else
                print_warning "环境变量 $var 未配置"
            fi
        done
    fi
    
    # 总结
    print_header "检查总结"
    echo -e "${CYAN}总检查项目: $total_checks${NC}"
    echo -e "${GREEN}通过检查: $passed_checks${NC}"
    echo -e "${RED}失败检查: $failed_checks${NC}"
    
    local success_rate=$((passed_checks * 100 / total_checks))
    echo -e "\n${CYAN}项目完整度: $success_rate%${NC}"
    
    if [ $success_rate -ge 90 ]; then
        echo -e "${GREEN}🎉 项目状态优秀！准备就绪可以投入使用。${NC}"
    elif [ $success_rate -ge 80 ]; then
        echo -e "${YELLOW}⚠️  项目状态良好，建议解决少量问题。${NC}"
    elif [ $success_rate -ge 70 ]; then
        echo -e "${YELLOW}⚠️  项目状态一般，需要解决一些问题。${NC}"
    else
        echo -e "${RED}❌ 项目状态不佳，需要解决大量问题。${NC}"
    fi
    
    echo -e "\n${CYAN}运行建议:${NC}"
    echo "1. 使用 ./test-start.sh 启动开发服务器"
    echo "2. 访问 http://localhost:3000 查看界面"
    echo "3. 查看 README.md 了解详细使用说明"
    echo "4. 查看 logs/ 目录中的日志文件排查问题"
    
    if [ $success_rate -lt 80 ]; then
        echo -e "\n${RED}建议在解决问题后重新运行此检查脚本。${NC}"
        exit 1
    fi
}

# 运行主函数
main "$@"
