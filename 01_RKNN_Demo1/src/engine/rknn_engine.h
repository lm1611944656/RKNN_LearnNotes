/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: rknn_engine.h
*   软件模块: engine
*   版 本 号: 1.0
*   生成日期: 2025-05-22
*   作    者: lium
*   功    能: RKNN推理引擎的实现类
* 
************************************************************************/

#include "engine.h"

class CRKEngine : public CEngine {
public:
    explicit CRKEngine();
    ~CRKEngine() override;   

public:
    /**加载模型 */
    TErrorTypes_e loadModel(const char *filename) override;

    /**获取模型输入的张量的属性 */
    const std::vector<TTensorAttr_s> getInputShape() override;

    /**获取模型输出的张量的属性 */
    const std::vector<TTensorAttr_s> getOutputShape() override;

    /**模型推理 */
    TErrorTypes_e run(std::vector<TTensorData_s> &inputs, std::vector<TTensorData_s> &outpus, bool want_float) override;
private:
};

/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-05-22, lium
* describe: 初始创建.
*************************************************************************/