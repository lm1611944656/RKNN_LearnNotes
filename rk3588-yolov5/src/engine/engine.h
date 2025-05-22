/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: engine.h
*   软件模块: engine
*   版 本 号: 1.0
*   生成日期: 2025-05-08
*   作    者: lium
*   功    能: 声明RKNN的抽象接口
* 
************************************************************************/

#ifndef RK3588_DEMO_ENGINE_H
#define RK3588_DEMO_ENGINE_H

#include <vector>
#include <memory>
#include "types/error.h"
#include "types/datatype.h"


/**
 * 1. 全部使用纯虚函数（=0），作用是将NNEngine定义为一个抽象类，不能实例化，只能作为基类使用
 * 2. 具体实现需要在子类中实现，这里的实现只是为了定义接口
 * 3. 用这种方式实现封装，可以使得不同的引擎的接口一致，方便使用；也可以隐藏不同引擎的实现细节，方便维护
 */
class NNEngine
{
public:
    virtual ~NNEngine(){};
    
    /**加载模型文件，=0表示纯虚函数，必须在子类中实现 */
    virtual nn_error_e LoadModelFile(const char *model_file) = 0;  
    
    /**获取输入张量的形状 */
    virtual const std::vector<tensor_attr_s> &GetInputShapes() = 0; 
    
    /**获取输出张量的形状 */
    virtual const std::vector<tensor_attr_s> &GetOutputShapes() = 0;  
    
    /**运行模型 */
    virtual nn_error_e Run(std::vector<tensor_data_s> &inputs, std::vector<tensor_data_s> &outpus, bool want_float) = 0; 
};

/**声明一个创建RKNN引擎对象的函数 */
std::shared_ptr<NNEngine> CreateRKNNEngine(); 

#endif /**RK3588_DEMO_ENGINE_H */

/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-05-08, lium
* describe: 初始创建.
*************************************************************************/
