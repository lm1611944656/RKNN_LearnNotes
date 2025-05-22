/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: preprocess.h
*   软件模块: process
*   版 本 号: 1.0
*   生成日期: 2025-05-08
*   作    者: lium
*   功    能: 前处理模块
* 
************************************************************************/
#ifndef RK3588_DEMO_PREPROCESS_H
#define RK3588_DEMO_PREPROCESS_H

#include <opencv2/opencv.hpp>
#include "types/datatype.h"

void imgPreprocess(const cv::Mat &img, cv::Mat &img_resized, uint32_t width, uint32_t height);
void cvimg2tensor(const cv::Mat &img, uint32_t width, uint32_t height, tensor_data_s &tensor);

#endif  /**RK3588_DEMO_PREPROCESS_H */

/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-05-08, lium
* describe: 初始创建.
*************************************************************************/

