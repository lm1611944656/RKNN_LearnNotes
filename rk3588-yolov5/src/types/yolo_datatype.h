#ifndef RK3588_DEMO_NN_DATATYPE_H
#define RK3588_DEMO_NN_DATATYPE_H

#include <opencv2/opencv.hpp>

typedef struct _nn_object_s {
    float x;
    float y;
    float w;
    float h;
    float score;
    int class_id;
} nn_object_s;


/**定义一个检测框信息结构体，使用c++11的风格进行初始化 */
struct Detection
{
    int class_id{0};                // 第几个类别
    std::string className{};        // 类别名称
    float confidence{0.0};          // 类别置信度
    cv::Scalar color{};             // 检测框颜色
    cv::Rect box{};                 // 检测框坐标
};

#endif //RK3588_DEMO_NN_DATATYPE_H
