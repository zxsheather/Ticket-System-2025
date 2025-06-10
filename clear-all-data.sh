#!/bin/bash

# 清理Ticket System 2025的所有数据
echo "正在清理Ticket System 2025的所有数据..."

# 进入项目根目录
cd /home/zxsheather/programming/ACMOJ/Ticket-System-2025

# 清理C++后端数据文件
echo "清理C++后端数据文件..."
rm -f *.block *.index *.memoryriver 2>/dev/null

# 清理Web前端日志文件
echo "清理Web前端日志文件..."
rm -rf web-frontend/logs/*.log 2>/dev/null
mkdir -p web-frontend/logs

# 清理构建产物（可选）
echo "清理构建产物..."
rm -rf build/*.o build/*.exe 2>/dev/null

# 显示清理结果
echo ""
echo "=== 清理完成 ==="
echo "已清理的文件类型："
echo "✅ .block 文件（用户、车次、订单等数据）"
echo "✅ .index 文件（索引文件）"
echo "✅ seat 文件（座位数据）"
echo "✅ 日志文件"
echo ""
echo "保留的文件："
echo "📁 源代码文件"
echo "📁 配置文件"
echo "📁 测试用例"
echo "📁 文档"
echo ""

# 检查剩余的数据文件
remaining_files=$(find . -maxdepth 1 -type f \( -name "*.block" -o -name "*.index" -o -name "seat" \) 2>/dev/null | wc -l)
if [ $remaining_files -eq 0 ]; then
    echo "🎉 所有数据文件已成功清理！"
else
    echo "⚠️  还有 $remaining_files 个数据文件未清理"
    find . -maxdepth 1 -type f \( -name "*.block" -o -name "*.index" -o -name "seat" \) 2>/dev/null
fi
