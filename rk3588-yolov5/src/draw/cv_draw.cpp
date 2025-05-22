/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: cv_draw.c
*   软件模块: draw
*   版 本 号: 1.0
*   生成日期: 2025-05-08
*   作    者: lium
*   功    能: 绘制目标检测信息
* 
************************************************************************/

#include "cv_draw.h"
#include "utils/logging.h"


void DrawDetections(cv::Mat &img, const std::vector<Detection> &objects)
{
    /**打印出有多少个检测目标 */
    NN_LOG_DEBUG("draw %ld objects", objects.size());

    for (const auto &object : objects)
    {
        /**绘制矩形框 */
        cv::rectangle(img, object.box, object.color, 2);
        
        /**绘制文本 */
        std::string draw_string = object.className + " " + std::to_string(object.confidence);
        cv::putText(img, draw_string, cv::Point(object.box.x, object.box.y - 5), cv::FONT_HERSHEY_SIMPLEX, 0.6,
                    object.color, 2);
    }
}

/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-05-08, lium
* describe: 初始创建.
*************************************************************************/