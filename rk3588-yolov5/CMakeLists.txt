# 设置最低版本号
cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

# 设置项目名称
project(rk3588-demo VERSION 0.0.1 LANGUAGES CXX)

# 输出系统信息
message(STATUS "System: ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_VERSION}")

# 设置编译器
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置库架构
set(LIB_ARCH "aarch64")
set(DEVICE_NAME "RK3588")

#  rknn_api 文件夹路径
set(RKNN_API_PATH ${CMAKE_CURRENT_SOURCE_DIR}/librknn_api)
#  rknn_api include 路径
set(RKNN_API_INCLUDE_PATH ${RKNN_API_PATH}/include)
#  rknn_api lib 路径
set(RKNN_API_LIB_PATH ${RKNN_API_PATH}/${LIB_ARCH}/librknnrt.so)

# 寻找本地的opencv
find_package(OpenCV REQUIRED)
# 输出OpenCV信息
message(STATUS "include path: ${OpenCV_INCLUDE_DIRS}")


# 用来搜索头文件的目录
include_directories(
    ${OpenCV_INCLUDE_DIRS}
    ${RKNN_API_INCLUDE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${RGA_DIR}/include
)

# 构建预处理和后处理库
add_library(nn_process SHARED
            src/process/preprocess.cpp
            src/process/postprocess.cpp
)
# 链接库
target_link_libraries(nn_process
    ${OpenCV_LIBS}
)


# 将自己封装的RKNN模块，生成(rknn_engine.so)动态库
add_library(rknn_engine SHARED src/engine/rknn_engine.cpp) 
# (rknn_engine.so)库需要链接RKNN的动态库
target_link_libraries(rknn_engine
    ${RKNN_API_LIB_PATH}
)

# 将自己封装的Yolov5模块，生成(yolov5_lib.so)动态库
add_library(yolov5_lib SHARED src/task/yolov5.cpp)
# (yolov5_lib.so)库需要链接RKNN的动态库
target_link_libraries(yolov5_lib
    rknn_engine
    nn_process
)

# draw_lib
add_library(draw_lib SHARED src/draw/cv_draw.cpp)
# 链接库
target_link_libraries(draw_lib
    ${OpenCV_LIBS}
)

# 测试自yolov5
add_executable(yolov5_test src/yolov5_test.cpp)

# 链接库
target_link_libraries(yolov5_test
        draw_lib
        yolov5_lib
)

# 测试yolov5_video
add_executable(yolov5_video src/yolov5_video.cpp)

# 链接库
target_link_libraries(yolov5_video
        draw_lib
        yolov5_lib
)