/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: preprocess.cpp
*   软件模块: process
*   版 本 号: 1.0
*   生成日期: 2025-05-08
*   作    者: lium
*   功    能: 前处理模块
* 
************************************************************************/

#include "preprocess.h"
#include "utils/logging.h"

/**
 * @brief 图像预处理函数：将输入图像转换为 RGB 格式并调整尺寸
 *
 * 该函数对输入的 OpenCV 图像进行预处理，包括：
 * - 检查图像是否为三通道（BGR）格式
 * - 将图像从 BGR 转换为 RGB 格式
 * - 缩放图像到指定的目标宽度和高度
 *
 * @param img 输入原始图像（必须是三通道 BGR 格式）
 * @param img_resized 输出参数，输出的处理后的图像（RGB 格式，已缩放）
 * @param width 输入参数，目标图像宽度
 * @param height 输入参数，目标图像高度
 */
void imgPreprocess(const cv::Mat &img, cv::Mat &img_resized, uint32_t width, uint32_t height)
{
    /**判断图像的通道数 */
    if (img.channels() != 3)
    {
        NN_LOG_ERROR("img has to be 3 channels");
        exit(-1);
    }

    cv::Mat img_rgb;
    cv::cvtColor(img, img_rgb, cv::COLOR_BGR2RGB);
    
    /**图像缩放(不建议这么处理) */
    cv::resize(img_rgb, img_resized, cv::Size(width, height), 0, 0, cv::INTER_LINEAR);
}

/**
 * @brief 将 OpenCV 图像转换为张量数据
 *
 * 该函数接收一个三通道（BGR）的 OpenCV 图像，并将其转换为指定大小的 RGB 图像，
 * 然后将处理后的图像数据复制到给定的张量结构中。这个过程包括：
 * - 检查输入图像是否为三通道
 * - 将图像从 BGR 转换为 RGB 格式
 * - 缩放图像到指定的目标宽度和高度
 * - 将缩放后的图像数据复制到张量数据区
 *
 * @param img 输入原始图像（必须是三通道 BGR 格式）
 * @param width 目标图像宽度
 * @param height 目标图像高度
 * @param tensor 输出张量数据结构，包含图像数据及其属性
 */
void cvimg2tensor(const cv::Mat &img, uint32_t width, uint32_t height, tensor_data_s &tensor)
{
    /**确保输入图像是三通道的（即 BGR 图像） */
    if (img.channels() != 3)
    {
        NN_LOG_ERROR("img has to be 3 channels");
        exit(-1);
    }

    /**BGR to RGB */
    cv::Mat img_rgb;
    cv::cvtColor(img, img_rgb, cv::COLOR_BGR2RGB);

    /**将 RGB 图像缩放到指定大小（width x height），使用线性插值方法 */
    cv::Mat img_resized;
    NN_LOG_DEBUG("img size: %d, %d", img.cols, img.rows);
    NN_LOG_DEBUG("resize to: %d, %d", width, height);
    cv::resize(img_rgb, img_resized, cv::Size(width, height), 0, 0, cv::INTER_LINEAR);
    
    /** 将调整大小后的图像数据复制到张量的数据区域; 注意：这里假设 tensor.data 已经分配了足够的空间来容纳 img_resized 的数据 */
    memcpy(tensor.data, img_resized.data, tensor.attr.size);
}

/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-05-08, lium
* describe: 初始创建.
*************************************************************************/
