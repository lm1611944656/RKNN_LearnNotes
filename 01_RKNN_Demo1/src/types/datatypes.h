/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: datatypes.h
*   软件模块: types
*   版 本 号: 1.0
*   生成日期: 2025-05-22
*   作    者: lium
*   功    能: 数据类型的声明
* 
************************************************************************/

#ifndef DATATYPES_H
#define DATATYPES_H

#include <stdlib.h>
#include <stdint.h>

/**定义张量布局枚举类型，用于描述张量数据的布局方式*/
typedef enum {
    NN_TENSORT_LAYOUT_UNKNOWN = 0, // 未知布局
    NN_TENSOR_NCHW = 1,            // NCHW布局：批量大小、通道数、高度、宽度
    NN_TENSOR_NHWC = 2,            // NHWC布局：批量大小、高度、宽度、通道数
    NN_TENSOR_OTHER = 3,           // 其他自定义布局
} TTensorLayout_e;

/**定义张量数据类型枚举，表示支持的数据类型*/
typedef enum {
    NN_TENSOR_INT8 = 1,     // 8位整数类型
    NN_TENSOR_UINT8 = 2,    // 无符号8位整数类型
    NN_TENSOR_FLOAT = 3,    // 单精度浮点数类型
    NN_TENSOR_FLOAT16 = 4,  // 半精度浮点数类型
} TTensorDatatype_e;

/**最大维度数，限制了张量的最大维度数量*/
static const int g_max_num_dims = 4;

/**张量属性结构体，包含关于张量的各种元数据信息
 * (图像的描述信息)*/
typedef struct
{
    unsigned int index;                     // 张量索引号
    unsigned int n_dims;                    // 维度的数量
    unsigned int dims[g_max_num_dims];      // 各个维度的大小
    unsigned int n_elems;                   // 总元素数量
    unsigned int size;                      // 数据总大小（以字节为单位）
    TTensorDatatype_e type;                 // 数据类型
    TTensorLayout_e layout;                 // 布局方式
    unsigned int zp;                        // 零点值（通常用于量化）
    float scale;                            // 缩放因子（通常用于量化）
} TTensorAttr_s;

/**张量数据结构体，不仅包含张量的属性，还包括实际的数据指针
 * (图像的描述信息 + 图像数据)*/
typedef struct
{
    TTensorAttr_s attr; // 张量的属性
    void *data;         // 指向实际数据的指针
} TTensorData_s;

size_t tensorDatatypeToSize(TTensorDatatype_e tensorDatatype);

/**
 * @brief 张量的属性转换为张量数据
 * @param 输入参数， 张量的属性
 * @param 输出参数， 张量数据(张量的属性 + 数据)
 */
void tensorAtrrToTensorData(const TTensorAttr_s &tensorAttr, TTensorData_s &tensorData);

#endif /**DATATYPES_H */

/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-05-08, lium
* describe: 初始创建.
*************************************************************************/