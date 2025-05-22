// 辅助函数

#ifndef RK3588_DEMO_ENGINE_HELPER_H
#define RK3588_DEMO_ENGINE_HELPER_H

#include <fstream>
#include <string.h>
#include <vector>
#include <rknn_api.h>
#include "utils/logging.h"
#include "types/datatype.h"

/**
 * @brief 加载模型文件到内存中
 *
 * 该函数以只读二进制方式打开指定的模型文件，将其内容读取到动态分配的内存中，
 * 并返回指向该内存的指针。同时通过输出参数返回模型数据的大小。
 *
 * @param filename 模型文件的路径名
 * @param model_size 输出参数，用于返回模型数据的字节大小
 * @return 成功时返回指向模型数据的指针；失败时返回 nullptr
 */
static unsigned char *load_model(const char *filename, int *model_size)
{
    /** 打开模型文件（以二进制只读模式） */
    FILE *fp = fopen(filename, "rb");
    if (fp == nullptr)
    {
        NN_LOG_ERROR("fopen %s fail!", filename);
        return nullptr;
    }

    /**将文件指针移动到文件末尾，用于获取文件长度 */
    fseek(fp, 0, SEEK_END);

    /**获取文件总长度（即模型大小） */
    int model_len = ftell(fp);

    /**在堆上分配足够大小的内存来保存模型数据 */
    unsigned char *model = (unsigned char *)malloc(model_len);

    /**将文件指针重新定位到文件开头 */
    fseek(fp, 0, SEEK_SET);

    /**读取整个模型文件到分配的内存中，并检查实际读取的字节数是否与预期一致 */
    if (model_len != fread(model, 1, model_len, fp))
    {
        NN_LOG_ERROR("fread %s fail!", filename);

        /**如果读取失败，释放已分配的内存 */
        free(model);
        return nullptr;
    }

    /**将模型大小写入输出参数 */
    *model_size = model_len;

    /**关闭文件句柄，防止资源泄漏 */
    if (fp)
    {
        fclose(fp);
    }

    /**返回指向模型数据的指针 */
    return model;
}

/**答应tensor的属性 */
static void print_tensor_attr(rknn_tensor_attr *attr)
{
    NN_LOG_INFO("  index=%d, name=%s, n_dims=%d, dims=[%d, %d, %d, %d], n_elems=%d, size=%d, fmt=%s, type=%s, qnt_type=%s, "
                "zp=%d, scale=%f",
                attr->index, attr->name, attr->n_dims, attr->dims[0], attr->dims[1], attr->dims[2], attr->dims[3],
                attr->n_elems, attr->size, get_format_string(attr->fmt), get_type_string(attr->type),
                get_qnt_type_string(attr->qnt_type), attr->zp, attr->scale);
}



static tensor_layout_e rknn_layout_convert(rknn_tensor_format fmt)
{
    switch (fmt)
    {
    case RKNN_TENSOR_NCHW:
        return NN_TENSOR_NCHW;
    case RKNN_TENSOR_NHWC:
        return NN_TENSOR_NHWC;
    default:
        return NN_TENSOR_OTHER;
    }
}

static rknn_tensor_format rknn_layout_convert(tensor_layout_e fmt)
{
    switch (fmt)
    {
    case NN_TENSOR_NCHW:
        return RKNN_TENSOR_NCHW;
    case NN_TENSOR_NHWC:
        return RKNN_TENSOR_NHWC;
    default:
        NN_LOG_ERROR("unsupported nn layout: %d\n", fmt);
        // exit program
        exit(1);
    }
}

static rknn_tensor_type rknn_type_convert(tensor_datatype_e type)
{
    switch (type)
    {
    case NN_TENSOR_UINT8:
        return RKNN_TENSOR_UINT8;
    case NN_TENSOR_FLOAT:
        return RKNN_TENSOR_FLOAT32;
    default:
        NN_LOG_ERROR("unsupported nn type: %d\n", type);
        // exit program
        exit(1);
    }
}

static tensor_datatype_e rknn_type_convert(rknn_tensor_type type)
{
    switch (type)
    {
    case RKNN_TENSOR_UINT8:
        return NN_TENSOR_UINT8;
    case RKNN_TENSOR_FLOAT32:
        return NN_TENSOR_FLOAT;
    case RKNN_TENSOR_INT8:
        return NN_TENSOR_INT8;
    case RKNN_TENSOR_FLOAT16:
        return NN_TENSOR_FLOAT16;
    default:
        NN_LOG_ERROR("unsupported rknn type: %d\n", type);
        // exit program
        exit(1);
    }
}

/**
 * @brief 将 rknn_tensor_attr 类型的张量属性转换为 tensor_attr_s 类型
 *
 * 该函数用于将 RKNN 框架中定义的张量属性结构体 rknn_tensor_attr 转换为
 * 应用程序自定义的张量属性结构体 tensor_attr_s，便于后续使用。
 *
 * @param attr 输入的 rknn_tensor_attr 类型的张量属性
 * @return 返回转换后的 tensor_attr_s 类型结构体
 */
static tensor_attr_s rknn_tensor_attr_convert(const rknn_tensor_attr &attr)
{
    /**定义目标结构体变量 shape 并初始化 */
    tensor_attr_s shape;

    /**复制维度数量 */
    shape.n_dims = attr.n_dims;

    /**复制张量索引 */
    shape.index = attr.index;

    /**复制各个维度的大小（逐个维度赋值） */
    for (int i = 0; i < attr.n_dims; ++i)
    {
        shape.dims[i] = attr.dims[i];
    }

    /**复制张量总字节数 */
    shape.size = attr.size;

    /**复制张量元素总个数 */
    shape.n_elems = attr.n_elems;
    
    /**设置张量布局（调用专用函数将格式从 rknn_tensor_format 转换为 layout_e） */
    shape.layout = rknn_layout_convert(attr.fmt);

    /**设置张量数据类型（调用专用函数将类型从 rknn_tensor_type 转换为 data_type_e） */
    shape.type = rknn_type_convert(attr.type);

    /**复制量化参数：零点（zero point） */
    shape.zp = attr.zp;

    /**复制量化参数：缩放因子（scale） */
    shape.scale = attr.scale;
    return shape;
}

static rknn_input tensor_data_to_rknn_input(const tensor_data_s &data)
{
    rknn_input input;
    memset(&input, 0, sizeof(input));
    // set default not passthrough
    input.index = data.attr.index;
    input.type = rknn_type_convert(data.attr.type);
    input.size = data.attr.size;
    input.fmt = rknn_layout_convert(data.attr.layout);
    input.buf = data.data;
    return input;
}

static void rknn_output_to_tensor_data(const rknn_output &output, tensor_data_s &data)
{
    data.attr.index = output.index;
    data.attr.size = output.size;
    NN_LOG_DEBUG("output size: %d", output.size);
    NN_LOG_DEBUG("output want_float: %d", output.want_float);
    memcpy(data.data, output.buf, output.size);
}

#endif // RK3588_DEMO_ENGINE_HELPER_H
