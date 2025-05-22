/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: rknn_engine.cpp
*   软件模块: engine
*   版 本 号: 1.0
*   生成日期: 2025-05-22
*   作    者: lium
*   功    能: RKNN推理引擎的实现类
* 
************************************************************************/

#include "rknn_engine.h"

CRKEngine::CRKEngine()
{

}

CRKEngine::~CRKEngine()
{

}

TErrorTypes_e CRKEngine::loadModel(const char *filename){
    return NN_SUCCESS;
}


const std::vector<TTensorAttr_s> CRKEngine::getInputShape() {
    return {};
}

const std::vector<TTensorAttr_s> CRKEngine::getOutputShape() {
    return {};  
}

TErrorTypes_e CRKEngine::run(std::vector<TTensorData_s> &inputs, std::vector<TTensorData_s> &outpus, bool want_float){
    return NN_SUCCESS;
}


/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-05-22, lium
* describe: 初始创建.
*************************************************************************/