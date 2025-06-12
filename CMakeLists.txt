cmake_minimum_required(VERSION 3.10)
project(Ticket-System-2025 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找依赖
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(PkgConfig QUIET)

# 设置可执行文件的输出名称为"code"
set(EXECUTABLE_NAME "code")

# 查找所有源文件
file(GLOB_RECURSE SOURCES "src/*.cpp")

# 分离命令行版本和Web版本的源文件
set(CLI_SOURCES ${SOURCES})
list(FILTER CLI_SOURCES EXCLUDE REGEX "src/web/")

set(WEB_SOURCES ${SOURCES})
list(FILTER WEB_SOURCES EXCLUDE REGEX "src/main.cpp")

# 查找所有头文件目录
file(GLOB_RECURSE HEADERS "src/*.hpp" "src/*.h")
set(INCLUDE_DIRS "")
foreach(HEADER ${HEADERS})
    get_filename_component(DIR ${HEADER} DIRECTORY)
    list(APPEND INCLUDE_DIRS ${DIR})
endforeach()
list(REMOVE_DUPLICATES INCLUDE_DIRS)

# 创建命令行版本可执行文件
add_executable(${EXECUTABLE_NAME} ${CLI_SOURCES})
target_include_directories(${EXECUTABLE_NAME} PRIVATE ${INCLUDE_DIRS})

# 创建Web版本可执行文件
if(EXISTS "${CMAKE_SOURCE_DIR}/src/web/server.cpp")
    add_executable(web_server ${WEB_SOURCES})
    target_include_directories(web_server PRIVATE ${INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/crow/include)
    target_link_libraries(web_server Threads::Threads OpenSSL::SSL OpenSSL::Crypto)
    target_compile_definitions(web_server PRIVATE CROW_ENABLE_SSL=0)
    
    # 设置Web服务器输出目录
    set_target_properties(web_server PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
endif()

# 添加编译选项
target_compile_options(${EXECUTABLE_NAME} PRIVATE -O2)

# 输出可执行文件到项目根目录
set_target_properties(${EXECUTABLE_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
)