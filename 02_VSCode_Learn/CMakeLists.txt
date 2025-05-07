# 设置CMake最低版本要求
cmake_minimum_required(VERSION 3.10)

# 定义项目名称和语言
project(application VERSION 1.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置构建类型为Debug，确保包含调试信息
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# 添加源文件目录
add_executable(${PROJECT_NAME} main.cpp ${PROJECT_SOURCE_DIR}/src/Object.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/src)
