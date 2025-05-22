/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: postprocess.h
*   软件模块: process
*   版 本 号: 1.0
*   生成日期: 2025-05-08
*   作    者: lium
*   功    能: 后处理模块
* 
************************************************************************/

#ifndef _RKNN_ZERO_COPY_DEMO_POSTPROCESS_H_
#define _RKNN_ZERO_COPY_DEMO_POSTPROCESS_H_

#include <stdint.h>
#include <vector>

#define OBJ_NAME_MAX_SIZE 16        // 对象名称(如类别名)的最大字符串长度
#define OBJ_NUMB_MAX_SIZE 64        // 单帧图像中可检测到的最大物体数量。
#define OBJ_CLASS_NUM 80            // 定义类别总数为80，适用于COCO数据集。如果是自定义模型，需要根据实际情况修改该值
#define NMS_THRESH 0.45             // 用于过滤掉重叠度较高的检测框，仅保留得分最高的那个框
#define BOX_THRESH 0.25             // 用于过滤掉置信度低于该阈值的检测框，减少误报

// 定义每个边界框所需的属性大小为 (5 + 类别总数)，包括中心坐标(x,y)、宽高(w,h)及置信度(score)，加上各类别的概率
#define PROP_BOX_SIZE (5 + OBJ_CLASS_NUM)   

namespace yolov5
{

    typedef struct _BOX_RECT
    {
        int left;
        int right;
        int top;
        int bottom;
    } BOX_RECT;

    typedef struct __detect_result_t
    {
        char name[OBJ_NAME_MAX_SIZE];
        BOX_RECT box;
        int id;
        float prop;
    } detect_result_t;

    typedef struct _detect_result_group_t
    {
        int id;
        int count;
        detect_result_t results[OBJ_NUMB_MAX_SIZE];
    } detect_result_group_t;

    int post_process(int8_t *input0, int8_t *input1, int8_t *input2, int model_in_h, int model_in_w,
                     float conf_threshold, float nms_threshold, float scale_w, float scale_h,
                     std::vector<int32_t> &qnt_zps, std::vector<float> &qnt_scales,
                     detect_result_group_t *group);

    void deinitPostProcess();
}
#endif /**_RKNN_ZERO_COPY_DEMO_POSTPROCESS_H_ */

/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-05-08, lium
* describe: 初始创建.
*************************************************************************/
