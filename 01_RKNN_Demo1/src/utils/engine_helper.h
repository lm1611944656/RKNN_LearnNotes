/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: engine_helper.h
*   软件模块: utils
*   版 本 号: 1.0
*   生成日期: 2025-05-22
*   作    者: lium
*   功    能: RKNN的辅助函数
* 
************************************************************************/
#ifndef ENGINE_HELPER_H
#define ENGINE_HELPER_H

#include <iostream>

/**
 * @brief 加载RKNN模型
 * @param modePath  输入参数，需要加载的模型文件路径;
 * @param modelSize 输出参数，加载的模型文件的大小；
 * @return 返回模型文件的内存地址
 */
unsigned char *loadModel(const char *modePath, int *modelSize);



#endif /**ENGINE_HELPER_H */

/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-05-08, lium
* describe: 初始创建.
*************************************************************************/