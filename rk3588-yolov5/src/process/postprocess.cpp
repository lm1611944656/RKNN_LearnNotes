/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: postprocess.cpp
*   软件模块: process
*   版 本 号: 1.0
*   生成日期: 2025-05-08
*   作    者: lium
*   功    能: 后处理模块
* 
************************************************************************/

#include "postprocess.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <set>
#include <vector>


namespace yolov5
{

#define LABEL_NALE_TXT_PATH "./src/coco_80_labels_list.txt"

    static const char *labels[OBJ_CLASS_NUM] = {
        "person", "bicycle", "car", "motorbike ", "aeroplane ", "bus ", "train", "truck ", "boat", "traffic light",
        "fire hydrant", "stop sign ", "parking meter", "bench", "bird", "cat", "dog ", "horse ", "sheep", "cow", "elephant",
        "bear", "zebra ", "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee", "skis", "snowboard", "sports ball", "kite",
        "baseball bat", "baseball glove", "skateboard", "surfboard", "tennis racket", "bottle", "wine glass", "cup", "fork", "knife ",
        "spoon", "bowl", "banana", "apple", "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza ", "donut", "cake", "chair", "sofa",
        "pottedplant", "bed", "diningtable", "toilet ", "tvmonitor", "laptop	", "mouse	", "remote ", "keyboard ", "cell phone", "microwave ",
        "oven ", "toaster", "sink", "refrigerator ", "book", "clock", "vase", "scissors ", "teddy bear ", "hair drier", "toothbrush "};

    const int anchor0[6] = {10, 13, 16, 30, 33, 23};
    const int anchor1[6] = {30, 61, 62, 45, 59, 119};
    const int anchor2[6] = {116, 90, 156, 198, 373, 326};

    /**
     * @brief 将一个浮点数值限制在指定的整数范围内（夹紧操作）
     *
     * 该函数用于将输入的浮点值 val 限制在一个最小值 min 和最大值 max 构成的闭区间 [min, max] 内。
     * 如果 val 小于 min，则返回 min；
     * 如果 val 大于 max，则返回 max；
     * 否则返回 val 本身。
     *
     * 使用 inline 关键字是为了建议编译器进行内联优化，适用于频繁调用的小函数。
     *
     * @param val 待限制的浮点数值
     * @param min 允许的最小整数值
     * @param max 允许的最大整数值
     * @return 返回夹紧后的整数值
     */
    inline static int clamp(float val, int min, int max) { 
        return val > min ? (val < max ? val : max) : min; 
    }

    /**
     * @brief 从文件指针中读取一行文本（动态分配内存）
     *
     * 该函数从文件指针 fp 中逐字符读取内容，直到遇到换行符 '\n' 或文件末尾 EOF，
     * 并将这一行内容存储在一个动态分配的缓冲区中返回。调用者需要在使用完后手动释放内存。
     *
     * @param fp 文件指针，指向要读取的文件流
     * @param buffer 输出参数：用于接收读取到的一行文本（实际为函数内部 malloc 的指针）
     * @param len 输出参数：用于返回读取到的字符串长度
     * @return 成功时返回指向动态分配缓冲区的指针；失败或文件结束时返回 NULL
     */
    char *readLine(FILE *fp, char *buffer, int *len)
    {
        int ch;                 // 当前读取的字符
        int i = 0;              // 已读取字符数计数器
        size_t buff_len = 0;    // 缓冲区长度（不包括结尾的 '\0'）        

        /**初始分配一个字节的空间（后续会逐步扩展） */
        buffer = (char *)malloc(buff_len + 1);
        if (!buffer)
            return NULL; // Out of memory

        /**循环读取字符，直到遇到换行符 '\n' 或文件末尾 EOF */
        while ((ch = fgetc(fp)) != '\n' && ch != EOF)
        {
            /**增加所需缓冲区长度 */
            buff_len++;
            
            /**重新分配内存以容纳新字符 */
            void *tmp = realloc(buffer, buff_len + 1);  /** 多分配一个字节用于 '\0' */
            if (tmp == NULL)
            {
                /**如果 realloc 失败，释放旧内存 */
                free(buffer);
                return NULL;
            }
            buffer = (char *)tmp;

            /**存储当前字符 */
            buffer[i] = (char)ch;
            i++;
        }

        /**添加字符串终止符 */
        buffer[i] = '\0';

        /**将实际读取的字符长度通过输出参数返回 */
        *len = buff_len;

        /**检查是否到达文件末尾且没有读取到有效数据 */
        if (ch == EOF && (i == 0 || ferror(fp)))
        {
            /**释放已分配的内存 */
            free(buffer);
            return NULL;
        }

        /**返回读取到的一行字符串 */
        return buffer;
    }

    /**从文件中读取多行数据 */
    int readLines(const char *fileName, char *lines[], int max_line)
    {
        FILE *file = fopen(fileName, "r");
        char *s;
        int i = 0;
        int n = 0;

        if (file == NULL)
        {
            printf("Open %s fail!\n", fileName);
            return -1;
        }

        while ((s = readLine(file, s, &n)) != NULL)
        {
            lines[i++] = s;
            if (i >= max_line)
                break;
        }
        fclose(file);
        return i;
    }

    int loadLabelName(const char *locationFilename, char *label[])
    {
        printf("loadLabelName %s\n", locationFilename);
        readLines(locationFilename, label, OBJ_CLASS_NUM);
        return 0;
    }

    /**
     * @brief 计算两个矩形框之间的交并比（IoU, Intersection over Union）
     *
     * 该函数用于计算两个轴对齐矩形框（bounding box）之间的交并比（IoU），常用于目标检测中的
     * 非极大值抑制（NMS）或评估预测框与真实框的重合度。
     *
     * 每个矩形由左上角 (xmin, ymin) 和右下角 (xmax, ymax) 表示。
     *
     * @param xmin0 第一个矩形的左上角 x 坐标
     * @param ymin0 第一个矩形的左上角 y 坐标
     * @param xmax0 第一个矩形的右下角 x 坐标
     * @param ymax0 第一个矩形的右下角 y 坐标
     * @param xmin1 第二个矩形的左上角 x 坐标
     * @param ymin1 第二个矩形的左上角 y 坐标
     * @param xmax1 第二个矩形的右下角 x 坐标
     * @param ymax1 第二个矩形的右下角 y 坐标
     * @return 返回两个矩形之间的 IoU 值，范围 [0, 1]
     */
    static float CalculateOverlap(
        float xmin0, float ymin0, float xmax0, float ymax0,
        float xmin1, float ymin1, float xmax1, float ymax1)
    {
        // 计算交集区域的宽度 w
        // 交集的左边界为两框左边界的最大值，右边界为两框右边界的最小值
        // 如果左边界 >= 右边界，则没有交集，宽度为 0
        float w = fmax(0.f, fmin(xmax0, xmax1) - fmax(xmin0, xmin1) + 1.0f);

        // 计算交集区域的高度 h
        // 上边界为两框上边界的最大值，下边界为两框下边界的最小值
        float h = fmax(0.f, fmin(ymax0, ymax1) - fmax(ymin0, ymin1) + 1.0f);

        // 交集面积 i = w * h
        float i = w * h;

        // 并集面积 u = A + B - i
        // A: 第一个矩形的面积
        // B: 第二个矩形的面积
        float area0 = (xmax0 - xmin0 + 1.0f) * (ymax0 - ymin0 + 1.0f);
        float area1 = (xmax1 - xmin1 + 1.0f) * (ymax1 - ymin1 + 1.0f);
        float u = area0 + area1 - i;

        // 如果并集面积为 0 或负数（无交集或无效区域），返回 IoU 为 0
        return u <= 0.f ? 0.f : (i / u); // 否则返回交并比：交集 / 并集
    }

    /**
     * @brief 使用非极大值抑制（NMS）去除重叠度较高的边界框
     *
     * 该函数对检测结果进行 NMS 后处理，保留置信度最高且与其他框重叠较小的边界框。
     * 主要用于目标检测模型输出后处理阶段。
     *
     * @param validCount 当前有效的检测框数量
     * @param outputLocations 检测框的位置信息数组，每个框包含 [x, y, width, height]
     * @param classIds 每个检测框对应的类别 ID
     * @param order 输入时为按置信度排序后的索引数组，输出时被抑制的框会被标记为 -1
     * @param filterId 要过滤的类别 ID，仅对该类别的检测框执行 NMS
     * @param threshold IoU 阈值，若两个框的 IoU 大于该阈值，则认为是重复框，将被抑制
     * @return 总是返回 0（表示成功）
     */
    static int nms(int validCount,
        std::vector<float> &outputLocations,
        std::vector<int> classIds,
        std::vector<int> &order,
        int filterId,
        float threshold)
    {
        // 遍历所有有效检测框
        for (int i = 0; i < validCount; ++i)
        {
            // 如果当前框无效（已被抑制）或不属于目标类别，则跳过
            if (order[i] == -1 || classIds[i] != filterId)
            {
                continue;
            }

            // 获取当前框在 outputLocations 中的真实索引
            int n = order[i];

            // 获取当前框的坐标：转换为左上角(xmin,ymin) + 右下角(xmax,ymax)
            float xmin0 = outputLocations[n * 4 + 0];                  // x
            float ymin0 = outputLocations[n * 4 + 1];                  // y
            float xmax0 = outputLocations[n * 4 + 0] + outputLocations[n * 4 + 2]; // x + width
            float ymax0 = outputLocations[n * 4 + 1] + outputLocations[n * 4 + 3]; // y + height

            // 对于当前框之后的所有框，检查是否与当前框有较大重叠
            for (int j = i + 1; j < validCount; ++j)
            {
                int m = order[j]; // 获取第 j 个框的真实索引

                // 如果该框已被抑制 或 不属于当前类别，则跳过
                if (m == -1 || classIds[m] != filterId)
                {
                    continue;
                }

                // 获取第 j 个框的坐标
                float xmin1 = outputLocations[m * 4 + 0];
                float ymin1 = outputLocations[m * 4 + 1];
                float xmax1 = outputLocations[m * 4 + 0] + outputLocations[m * 4 + 2];
                float ymax1 = outputLocations[m * 4 + 1] + outputLocations[m * 4 + 3];

                // 计算两个框之间的交并比（IoU）
                float iou = CalculateOverlap(xmin0, ymin0, xmax0, ymax0, xmin1, ymin1, xmax1, ymax1);

                // 如果 IoU 超过阈值，说明这两个框太相似，抑制第 j 个框
                if (iou > threshold)
                {
                    order[j] = -1; // 标记为已抑制
                }
            }
        }
        return 0; // 返回成功
    }

    static int quick_sort_indice_inverse(std::vector<float> &input, int left, int right, std::vector<int> &indices)
    {
        float key;
        int key_index;
        int low = left;
        int high = right;
        if (left < right)
        {
            key_index = indices[left];
            key = input[left];
            while (low < high)
            {
                while (low < high && input[high] <= key)
                {
                    high--;
                }
                input[low] = input[high];
                indices[low] = indices[high];
                while (low < high && input[low] >= key)
                {
                    low++;
                }
                input[high] = input[low];
                indices[high] = indices[low];
            }
            input[low] = key;
            indices[low] = key_index;
            quick_sort_indice_inverse(input, left, low - 1, indices);
            quick_sort_indice_inverse(input, low + 1, right, indices);
        }
        return low;
    }

    static float sigmoid(float x) { 
        return 1.0 / (1.0 + expf(-x)); 
    }

    static float unsigmoid(float y) { 
        return -1.0 * logf((1.0 / y) - 1.0); 
    }

    inline static int32_t __clip(float val, float min, float max)
    {
        float f = val <= min ? min : (val >= max ? max : val);
        return f;
    }

    static int8_t qnt_f32_to_affine(float f32, int32_t zp, float scale)
    {
        float dst_val = (f32 / scale) + zp;
        int8_t res = (int8_t)__clip(dst_val, -128, 127);
        return res;
    }

    static float deqnt_affine_to_f32(int8_t qnt, int32_t zp, float scale) { return ((float)qnt - (float)zp) * scale; }

    static int process(int8_t *input, int *anchor, int grid_h, int grid_w, int height, int width, int stride,
                       std::vector<float> &boxes, std::vector<float> &objProbs, std::vector<int> &classId,
                       float threshold,
                       int32_t zp, float scale)
    {
        int validCount = 0;
        int grid_len = grid_h * grid_w;
        float thres = unsigmoid(threshold);
        int8_t thres_i8 = qnt_f32_to_affine(thres, zp, scale);
        for (int a = 0; a < 3; a++)
        {
            for (int i = 0; i < grid_h; i++)
            {
                for (int j = 0; j < grid_w; j++)
                {
                    int8_t box_confidence = input[(PROP_BOX_SIZE * a + 4) * grid_len + i * grid_w + j];
                    if (box_confidence >= thres_i8)
                    {
                        int offset = (PROP_BOX_SIZE * a) * grid_len + i * grid_w + j;
                        int8_t *in_ptr = input + offset;
                        float box_x = sigmoid(deqnt_affine_to_f32(*in_ptr, zp, scale)) * 2.0 - 0.5;
                        float box_y = sigmoid(deqnt_affine_to_f32(in_ptr[grid_len], zp, scale)) * 2.0 - 0.5;
                        float box_w = sigmoid(deqnt_affine_to_f32(in_ptr[2 * grid_len], zp, scale)) * 2.0;
                        float box_h = sigmoid(deqnt_affine_to_f32(in_ptr[3 * grid_len], zp, scale)) * 2.0;
                        box_x = (box_x + j) * (float)stride;
                        box_y = (box_y + i) * (float)stride;
                        box_w = box_w * box_w * (float)anchor[a * 2];
                        box_h = box_h * box_h * (float)anchor[a * 2 + 1];
                        box_x -= (box_w / 2.0);
                        box_y -= (box_h / 2.0);

                        int8_t maxClassProbs = in_ptr[5 * grid_len];
                        int maxClassId = 0;
                        for (int k = 1; k < OBJ_CLASS_NUM; ++k)
                        {
                            int8_t prob = in_ptr[(5 + k) * grid_len];
                            if (prob > maxClassProbs)
                            {
                                maxClassId = k;
                                maxClassProbs = prob;
                            }
                        }
                        if (maxClassProbs > thres_i8)
                        {
                            objProbs.push_back(sigmoid(deqnt_affine_to_f32(maxClassProbs, zp, scale)) *
                                               sigmoid(deqnt_affine_to_f32(box_confidence, zp, scale)));
                            classId.push_back(maxClassId);
                            validCount++;
                            boxes.push_back(box_x);
                            boxes.push_back(box_y);
                            boxes.push_back(box_w);
                            boxes.push_back(box_h);
                        }
                    }
                }
            }
        }
        return validCount;
    }

    int
    post_process(int8_t *input0, int8_t *input1, int8_t *input2, int model_in_h, int model_in_w, float conf_threshold,
                 float nms_threshold, float scale_w, float scale_h, std::vector<int32_t> &qnt_zps,
                 std::vector<float> &qnt_scales, detect_result_group_t *group)
    {
        static int init = -1;
        if (init == -1)
        {
            int ret = 0;
            //            ret = loadLabelName(LABEL_NALE_TXT_PATH, labels);
            if (ret < 0)
            {
                return -1;
            }

            init = 0;
        }
        memset(group, 0, sizeof(detect_result_group_t));

        std::vector<float> filterBoxes;
        std::vector<float> objProbs;
        std::vector<int> classId;

        // stride 8
        int stride0 = 8;
        int grid_h0 = model_in_h / stride0;
        int grid_w0 = model_in_w / stride0;
        int validCount0 = 0;
        validCount0 = process(input0, (int *)anchor0, grid_h0, grid_w0, model_in_h, model_in_w, stride0, filterBoxes,
                              objProbs,
                              classId, conf_threshold, qnt_zps[0], qnt_scales[0]);

        // stride 16
        int stride1 = 16;
        int grid_h1 = model_in_h / stride1;
        int grid_w1 = model_in_w / stride1;
        int validCount1 = 0;
        validCount1 = process(input1, (int *)anchor1, grid_h1, grid_w1, model_in_h, model_in_w, stride1, filterBoxes,
                              objProbs,
                              classId, conf_threshold, qnt_zps[1], qnt_scales[1]);

        // stride 32
        int stride2 = 32;
        int grid_h2 = model_in_h / stride2;
        int grid_w2 = model_in_w / stride2;
        int validCount2 = 0;
        validCount2 = process(input2, (int *)anchor2, grid_h2, grid_w2, model_in_h, model_in_w, stride2, filterBoxes,
                              objProbs,
                              classId, conf_threshold, qnt_zps[2], qnt_scales[2]);

        int validCount = validCount0 + validCount1 + validCount2;
        // no object detect
        if (validCount <= 0)
        {
            return 0;
        }

        std::vector<int> indexArray;
        for (int i = 0; i < validCount; ++i)
        {
            indexArray.push_back(i);
        }

        quick_sort_indice_inverse(objProbs, 0, validCount - 1, indexArray);

        std::set<int> class_set(std::begin(classId), std::end(classId));

        for (auto c : class_set)
        {
            nms(validCount, filterBoxes, classId, indexArray, c, nms_threshold);
        }

        int last_count = 0;
        group->count = 0;
        /* box valid detect target */
        for (int i = 0; i < validCount; ++i)
        {
            if (indexArray[i] == -1 || last_count >= OBJ_NUMB_MAX_SIZE)
            {
                continue;
            }
            int n = indexArray[i];

            float x1 = filterBoxes[n * 4 + 0];
            float y1 = filterBoxes[n * 4 + 1];
            float x2 = x1 + filterBoxes[n * 4 + 2];
            float y2 = y1 + filterBoxes[n * 4 + 3];
            int id = classId[n];
            float obj_conf = objProbs[i];

            group->results[last_count].box.left = (int)(clamp(x1, 0, model_in_w) / scale_w);
            group->results[last_count].box.top = (int)(clamp(y1, 0, model_in_h) / scale_h);
            group->results[last_count].box.right = (int)(clamp(x2, 0, model_in_w) / scale_w);
            group->results[last_count].box.bottom = (int)(clamp(y2, 0, model_in_h) / scale_h);
            group->results[last_count].prop = obj_conf;
            const char *label = labels[id];
            strncpy(group->results[last_count].name, label, OBJ_NAME_MAX_SIZE);

            // printf("result %2d: (%4d, %4d, %4d, %4d), %s\n", i, group->results[last_count].box.left,
            // group->results[last_count].box.top,
            //        group->results[last_count].box.right, group->results[last_count].box.bottom, label);
            last_count++;
        }
        group->count = last_count;

        return 0;
    }

    void deinitPostProcess()
    {
        //        for (int i = 0; i < OBJ_CLASS_NUM; i++) {
        //            if (labels[i] != nullptr) {
        //                free(labels[i]);
        //                labels[i] = nullptr;
        //            }
        //        }
    }

}

/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-05-08, lium
* describe: 初始创建.
*************************************************************************/