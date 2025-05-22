/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: cv_draw.h
*   软件模块: draw
*   版 本 号: 1.0
*   生成日期: 2025-05-08
*   作    者: lium
*   功    能: 绘制目标检测信息
* 
************************************************************************/

#ifndef RK3588_DEMO_CV_DRAW_H
#define RK3588_DEMO_CV_DRAW_H

#include <opencv2/opencv.hpp>
#include "types/yolo_datatype.h"

/**
 * @brief 在目标上绘制检测框
 * @param img       输入参数，需要绘制目标检测框的图像
 * @param objects   输入参数，检查框信息
 */
void DrawDetections(cv::Mat& img, const std::vector<Detection>& objects);

#endif /**RK3588_DEMO_CV_DRAW_H */

/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-05-08, lium
* describe: 初始创建.
*************************************************************************/
