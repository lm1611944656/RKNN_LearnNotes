/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: error.h
*   软件模块: types
*   版 本 号: 1.0
*   生成日期: 2025-05-22
*   作    者: lium
*   功    能: 错误类型的声明
* 
************************************************************************/

#ifndef ERROR_TYPES_H
#define ERROR_TYPES_H

typedef enum{
    NN_SUCCESS = 0,                 // 成功
    NN_LOAD_MODEL_FAIL = -1,        // 加载模型失败
    NN_RKNN_INIT_FAIL = -2,         // rknn初始化失败
    NN_RKNN_QUERY_FAIL = -3,        // rknn查询失败
    NN_RKNN_INPUT_SET_FAIL = -4,    // rknn设置输入数据失败
    NN_RKNN_RUNTIME_ERROR = -5,     // rknn运行时错误
    NN_IO_NUM_NOT_MATCH = -6,       // 输入输出数量不匹配
    NN_RKNN_OUTPUT_GET_FAIL = -7,   // rknn获取输出数据失败
    NN_RKNN_INPUT_ATTR_ERROR = -8,  // rknn输入数据属性错误
    NN_RKNN_OUTPUT_ATTR_ERROR = -9, // rknn输出数据属性错误
    NN_RKNN_MODEL_NOT_LOAD = -10,   // rknn模型未加载
} TErrorTypes_e;

#endif /**ERROR_TYPES_H */

/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-05-08, lium
* describe: 初始创建.
*************************************************************************/