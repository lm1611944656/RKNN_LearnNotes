# 设置最低版本号
cmake_minimum_required(VERSION 3.11 FATAL_ERROR)
# 设置项目名称
project(yunyan VERSION 0.0.1 LANGUAGES CXX)

# 输出系统信息
message(STATUS "System: ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_VERSION}")

# 设置编译器
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置库架构
set(LIB_ARCH "aarch64")
set(DEVICE_NAME "RK3588")

# 
link_directories(
    submodules_libs
)
#  rknn_api 文件夹路径
set(RKNN_API_PATH ${CMAKE_CURRENT_SOURCE_DIR}/librknn_api)
#  rknn_api include 路径
set(RKNN_API_INCLUDE_PATH ${RKNN_API_PATH}/include)
#  rknn_api lib 路径
set(RKNN_API_LIB_PATH ${RKNN_API_PATH}/${LIB_ARCH}/librknnrt.so)

# 寻找OpenCV库，使用自定义的OpenCV_DIR
set(3RDPARTY_PATH ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty)
set(RGA_DIR ${3RDPARTY_PATH}/rga/${DEVICE_NAME})
set(RGA_LIB ${RGA_DIR}/lib/Linux/${LIB_ARCH}/librga.so)
find_package(OpenCV REQUIRED)
# 输出OpenCV信息
message(STATUS "include path: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "${OpenCV_VERSION}")

# 用来搜索头文件的目录
include_directories(
    ${OpenCV_INCLUDE_DIRS}
    ${RKNN_API_INCLUDE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/submodules/ZLMediaKit/api/include
    ${RGA_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/submodules/mpp/inc
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

# 构建自定义封装API库
add_library(rknn_engine SHARED src/engine/rknn_engine.cpp) 
# 链接库
target_link_libraries(rknn_engine
    ${RKNN_API_LIB_PATH}
)
# yolov5_lib
add_library(yolov5_lib SHARED src/task/yolov5.cpp )
# 链接库
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

# reconfig用来读取ini配置文件
add_library(reconfig_lib SHARED 
    src/reconfig/ReConfig.cpp
)

# img_demo
add_executable(img_demo
    src/img_demo.cpp
)

target_link_libraries(img_demo
    rockchip_mpp
    draw_lib
    yolov5_lib
)
# img_demo



# thread_pool_demo
add_executable(thread_pool_demo
    src/thread_pool_demo.cpp
    src/task/yolov5_thread_pool.cpp
)

target_link_libraries(thread_pool_demo
    draw_lib
    yolov5_lib
)
# thread_pool_demo

# rtsp_to_rtsp_demo
add_executable(rtsp_to_rtsp_demo
    src/rtsp_to_rtsp_demo.cpp
    src/task/yolov5_thread_pool.cpp
    src/rkmedia/utils/mpp_decoder.cpp
    src/rkmedia/utils/mpp_encoder.cpp
)

target_link_libraries(rtsp_to_rtsp_demo
    rockchip_mpp
    mk_api
    draw_lib
    yolov5_lib
    reconfig_lib # 自己补充的一些方法很内容主要是读取配置文件，射线法判断穿越
    ${RGA_LIB}
)
# rtsp_to_rtsp_demo