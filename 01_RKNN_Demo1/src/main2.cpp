#include <iostream>
#include "engine_helper.h"

std::string imgPath = "images/bus.jpg";


int main(int argc, char **argv)
{
    const char *model_file = "weigths/yolov5s.rknn";
    const char *img_file = "images/bus.jpg";
    
    int modelSize;
    unsigned char *model  = loadModel(model_file, &modelSize);
    std::cout << "模型的大小为：" << modelSize << std::endl;
    if(model == NULL){
        std::cout << "读取模型失败" << std::endl;
    }
    return 0;  
}