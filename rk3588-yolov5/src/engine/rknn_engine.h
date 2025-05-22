/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: rknn_engine.h
*   软件模块: engine
*   版 本 号: 1.0
*   生成日期: 2025-05-08
*   作    者: lium
*   功    能: 继承自NNEngine，实现NNEngine的接口
* 
************************************************************************/

#ifndef RK3588_DEMO_RKNN_ENGINE_H
#define RK3588_DEMO_RKNN_ENGINE_H

#include <vector>
#include <rknn_api.h>
#include "engine.h"

class RKEngine : public NNEngine
{
public:
    //RKEngine() : rknn_ctx_(0), ctx_created_(false), input_num_(0), output_num_(0){}; // 构造函数，初始化
    explicit RKEngine();
    ~RKEngine() override;                                                            

    /**加载模型文件 */
    nn_error_e LoadModelFile(const char *model_file) override;                                                         
    
    /**获取输入张量的形状 */
    const std::vector<tensor_attr_s> &GetInputShapes() override;                                                       
    
    /**获取输出张量的形状 */
    const std::vector<tensor_attr_s> &GetOutputShapes() override;                                                      
    
    /**运行模型 */
    nn_error_e Run(std::vector<tensor_data_s> &inputs, std::vector<tensor_data_s> &outputs, bool want_float) override; 

private:
    /**rknn context */
    rknn_context rknn_ctx_; 

    /**rknn context是否创建标志位 */
    bool ctx_created_;      

    uint32_t input_num_;  // 输入的数量
    uint32_t output_num_; // 输出的数量

    /**输入张量的形状 */
    std::vector<tensor_attr_s> in_shapes_;  

    /**输出张量的形状 */
    std::vector<tensor_attr_s> out_shapes_; 
};

#endif /**RK3588_DEMO_RKNN_ENGINE_H */

/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-05-08, lium
* describe: 初始创建.
*************************************************************************/