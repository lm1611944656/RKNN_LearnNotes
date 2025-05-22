/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: yolov5_test.cpp
*   软件模块: 主函数
*   版 本 号: 1.0
*   生成日期: 2025-05-08
*   作    者: lium
*   功    能: RKNN模型推理
* 
************************************************************************/

#include <opencv2/opencv.hpp>
#include "task/yolov5.h"
#include "utils/logging.h"
#include "draw/cv_draw.h"

int main(int argc, char **argv)
{
    // model file path
    const char *model_file = argv[1];
    // input img path
    const char *img_file = argv[2];
    // 读取图片
    cv::Mat img = cv::imread(img_file);

    // 初始化
    Yolov5 yolo;
    // 加载模型
    yolo.LoadModel(model_file);

    /**执行目标检测 */
    std::vector<Detection> objects;
    yolo.Run(img, objects);

    /**绘制结果 */
    DrawDetections(img, objects);

    /**保存结果 */
    cv::imwrite("result.jpg", img);

    return 0;
}

/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-05-08, lium
* describe: 初始创建.
*************************************************************************/