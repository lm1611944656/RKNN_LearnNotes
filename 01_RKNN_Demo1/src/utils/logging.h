/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: logging.h
*   软件模块: utils
*   版 本 号: 1.0
*   生成日期: 2025-05-08
*   作    者: lium
*   功    能: 日志信息的声明
* 
************************************************************************/

#ifndef LOGGING_H
#define LOGGING_H

// a logging wrapper so it can be easily replaced
#include <stdio.h>


/**
 * @brief 日志等级设置
 * 
 * 0: no log                        没有任何日志
 * 1: error                         只是打印错误日志
 * 2: error, warning                错误 + 警告
 * 3: error, warning, info          错误 + 警告 + 信息 
 * 4: error, warning, info, debug   错误 + 警告 + 信息 + 调试
 */
static unsigned int g_log_level = 4;

// a printf wrapper so the msg can be formatted with %d %s, etc.

#define NN_LOG_ERROR(...)          \
    do                             \
    {                              \
        if (g_log_level >= 1)      \
        {                          \
            printf("[NN_ERROR] "); \
            printf(__VA_ARGS__);   \
            printf("\n");          \
        }                          \
    } while (0)

#define NN_LOG_WARNING(...)          \
    do                               \
    {                                \
        if (g_log_level >= 2)        \
        {                            \
            printf("[NN_WARNING] "); \
            printf(__VA_ARGS__);     \
            printf("\n");            \
        }                            \
    } while (0)

#define NN_LOG_INFO(...)          \
    do                            \
    {                             \
        if (g_log_level >= 3)     \
        {                         \
            printf("[NN_INFO] "); \
            printf(__VA_ARGS__);  \
            printf("\n");         \
        }                         \
    } while (0)

#define NN_LOG_DEBUG(...)          \
    do                             \
    {                              \
        if (g_log_level >= 4)      \
        {                          \
            printf("[NN_DEBUG] "); \
            printf(__VA_ARGS__);   \
            printf("\n");          \
        }                          \
    } while (0)

#endif /**LOGGING_H */

/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-05-08, lium
* describe: 初始创建.
*************************************************************************/