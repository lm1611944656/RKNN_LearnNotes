/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: datatypes.h
*   软件模块: types
*   版 本 号: 1.0
*   生成日期: 2025-05-22
*   作    者: lium
*   功    能: 数据类型的定义
* 
************************************************************************/

#include "datatypes.h"
#include "logging.h"

size_t tensorDatatypeToSize(TTensorDatatype_e tensorDatatype){
    switch (tensorDatatype)
    {
    case NN_TENSOR_INT8:
        return (sizeof(signed char));
    case NN_TENSOR_UINT8:
        return (sizeof(unsigned char));
    case NN_TENSOR_FLOAT:
        return (sizeof(float));
    case NN_TENSOR_FLOAT16:
        return (sizeof(uint16_t));
    default:
        NN_LOG_ERROR("unsupported tensor type");
        return 0;
    }
}

/**
 * @brief 张量的属性转换为张量数据
 * @param 输入参数， 张量的属性
 * @param 输出参数， 张量数据(张量的属性 + 数据)
 */
void tensorAtrrToTensorData(const TTensorAttr_s &tensorAttr, TTensorData_s &tensorData) {
    if (tensorAttr.n_dims != 4)
    {
        NN_LOG_ERROR("unsupported input dims");
        exit(-1);
    }

    tensorData.attr.index = 0;
    tensorData.attr.n_dims = tensorAttr.n_dims;
    tensorData.attr.type = NN_TENSOR_UINT8;
    tensorData.attr.layout = NN_TENSOR_NHWC;
    if (tensorAttr.layout == NN_TENSOR_NCHW)
    {
        tensorData.attr.dims[0] = tensorAttr.dims[0];
        tensorData.attr.dims[1] = tensorAttr.dims[2];
        tensorData.attr.dims[2] = tensorAttr.dims[3];
        tensorData.attr.dims[3] = tensorAttr.dims[1];
    }
    else if (tensorAttr.layout == NN_TENSOR_NHWC)
    {
        tensorData.attr.dims[0] = tensorAttr.dims[0];
        tensorData.attr.dims[1] = tensorAttr.dims[1];
        tensorData.attr.dims[2] = tensorAttr.dims[2];
        tensorData.attr.dims[3] = tensorAttr.dims[3];
    }
    else
    {
        NN_LOG_ERROR("unsupported input layout");
        exit(-1);
    }
    tensorData.attr.n_elems = tensorData.attr.dims[0] * tensorData.attr.dims[1] *
                        tensorData.attr.dims[2] * tensorData.attr.dims[3];
    tensorData.attr.size = tensorData.attr.n_elems * sizeof(uint8_t);
}


/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-05-08, lium
* describe: 初始创建.
*************************************************************************/