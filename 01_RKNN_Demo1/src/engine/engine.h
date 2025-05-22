/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: engine.h
*   软件模块: engine
*   版 本 号: 1.0
*   生成日期: 2025-05-22
*   作    者: lium
*   功    能: RKNN推理引擎的接口类
* 
************************************************************************/

#include <iostream>
#include <vector>
#include "error_types.h"
#include "datatypes.h"

class CEngine{
public:
    CEngine() = default;
    virtual ~CEngine() = default;
    
public:
    /**加载模型 */
    virtual TErrorTypes_e loadModel(const char *filename) = 0;

    /**获取模型输入的张量的属性 */
    virtual const std::vector<TTensorAttr_s> getInputShape() = 0;

    /**获取模型输出的张量的属性 */
    virtual const std::vector<TTensorAttr_s> getOutputShape() = 0;

    /**模型推理 */
    virtual TErrorTypes_e run(std::vector<TTensorData_s> &inputs, std::vector<TTensorData_s> &outpus, bool want_float) = 0;
};


/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-05-22, lium
* describe: 初始创建.
*************************************************************************/