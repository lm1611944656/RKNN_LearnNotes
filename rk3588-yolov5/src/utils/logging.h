/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: logging.h
*   软件模块: utils
*   版 本 号: 1.0
*   生成日期: 2025-05-08
*   作    者: lium
*   功    能: 日志信息的定义
* 
************************************************************************/

#ifndef RK3588_DEMO_LOGGING_H
#define RK3588_DEMO_LOGGING_H

#include <stdio.h>


/**
 * @brief log level from low to high
 * 0: no log
 * 1: error
 * 2: error, warning
 * 3: error, warning, info
 * 4: error, warning, info, debug
 */
static int32_t g_log_level = 4;

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

#endif  /**RK3588_DEMO_LOGGING_H */

/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-05-08, lium
* describe: 初始创建.
*************************************************************************/
