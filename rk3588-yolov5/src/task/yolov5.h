/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: yolov5.h
*   软件模块: 利用yolo模型推理
*   版 本 号: 1.0
*   生成日期: 2025-05-08
*   作    者: lium
*   功    能: RKNN模型推理
* 
************************************************************************/

#ifndef RK3588_DEMO_YOLOV5_H
#define RK3588_DEMO_YOLOV5_H

#include "types/yolo_datatype.h"
#include "engine/engine.h"

class Yolov5
{
public:
    Yolov5();
    ~Yolov5();

    /**加载模型 */
    nn_error_e LoadModel(const char *model_path);        
    
    /** 目标检测 */
    nn_error_e Run(const cv::Mat &img, std::vector<Detection> &objects);

private:
    /**图像预处理 */
    nn_error_e Preprocess(const cv::Mat &img);    
    
    /**推理 */
    nn_error_e Inference();     
    
    /**后处理 */
    nn_error_e Postprocess(const cv::Mat &img, std::vector<Detection> &objects);

    tensor_data_s input_tensor_;
    std::vector<tensor_data_s> output_tensors_;
    std::vector<int32_t> out_zps_;
    std::vector<float> out_scales_;
    std::shared_ptr<NNEngine> engine_;
};

#endif /**RK3588_DEMO_YOLOV5_H */

/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-05-08, lium
* describe: 初始创建.
*************************************************************************/
