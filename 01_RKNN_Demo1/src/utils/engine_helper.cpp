/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: engine_helper.cpp
*   软件模块: utils
*   版 本 号: 1.0
*   生成日期: 2025-05-22
*   作    者: lium
*   功    能: RKNN的辅助函数
* 
************************************************************************/

#include "engine_helper.h"
#include "logging.h"
#include <cstring>

unsigned char *loadModel(const char *filename, int *model_size)
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



/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-05-08, lium
* describe: 初始创建.
*************************************************************************/