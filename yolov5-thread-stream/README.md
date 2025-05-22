# 本项目需要先编译安装mpp、zlmediakit

## zlmediakit的安装编译
cd submodules
git clone https://gitee.com/tangerine_yun/ZLMediaKit.git
cd ZLMediaKit
git submodule init
git submodule update
cmake . -B build && cmake --build build -j4

## mpp的安装编译
cd submodules
git clone https://gitee.com/tangerine_yun/mpp.git
cd mpp
cmake . -B build && cmake --build build -j4

## 讲包移动到项目目录中，方便后期的编译处理
cp ./submodules/mpp/build/utils/libutils.a submodules_libs/
cp ./submodules/ZLMediaKit/release/linux/Debug/libmk_api.so submodules_libs/
