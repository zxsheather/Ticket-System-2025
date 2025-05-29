cmake_minimum_required(VERSION 3.10)
project(Ticket-System-2025 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置可执行文件的输出名称为"code"
set(EXECUTABLE_NAME "code")

# 查找所有源文件
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
)

# 查找所有头文件目录
file(GLOB_RECURSE HEADERS "src/*.hpp" "src/*.h")
set(INCLUDE_DIRS "")
foreach(HEADER ${HEADERS})
    get_filename_component(DIR ${HEADER} DIRECTORY)
    list(APPEND INCLUDE_DIRS ${DIR})
endforeach()
list(REMOVE_DUPLICATES INCLUDE_DIRS)

# 包含所有头文件目录
include_directories(${INCLUDE_DIRS})

# 创建可执行文件
add_executable(${EXECUTABLE_NAME} ${SOURCES})

# 添加编译选项（根据需要调整）
target_compile_options(${EXECUTABLE_NAME} PRIVATE
    # -Wall                # 启用所有警告
    # -Wextra              # 启用额外警告
    -Ofast                # 优化级别
)
# 输出可执行文件到项目根目录
set_target_properties(${EXECUTABLE_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
)