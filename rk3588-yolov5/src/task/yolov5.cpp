/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: yolov5.h
*   软件模块: 利用yolo模型推理
*   版 本 号: 1.0
*   生成日期: 2025-05-08
*   作    者: lium
*   功    能: RKNN模型推理
* 
************************************************************************/

#include "yolov5.h"
#include <memory>
#include "utils/logging.h"
#include "process/preprocess.h"
#include "process/postprocess.h"
#include <ctime>


Yolov5::Yolov5()
{
    /**创建一个RKNN的引擎对象 */
    engine_ = CreateRKNNEngine();
    input_tensor_.data = nullptr;
}

Yolov5::~Yolov5()
{
    if (input_tensor_.data != nullptr)
    {
        free(input_tensor_.data);
        input_tensor_.data = nullptr;
    }
    for (auto &tensor : output_tensors_)
    {
        free(tensor.data);
        tensor.data = nullptr;
    }
}

/**
 * @brief 将 yolov5::detect_result_group_t 类型的检测结果转换为 Detection 结构体数组
 *
 * 该函数将 YOLOv5 模型输出的检测结果（通常是一个结构化的分组结果）转换为一个
 * std::vector<Detection> 类型的容器，便于后续在图像上绘制边界框、类别名等信息。
 * 同时为每个检测对象随机生成颜色，用于可视化显示。
 *
 * @param det_grp 输入参数，包含模型输出的检测结果组
 * @param objects 输出参数，用于存储转换后的 Detection 对象列表
 */
void DetectionGrp2DetectionArray(yolov5::detect_result_group_t &det_grp, std::vector<Detection> &objects)
{
    /**使用当前系统时间作为随机数种子，确保每次运行时生成的颜色不同 */
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    /**遍历所有检测到的对象 */
    for (int i = 0; i < det_grp.count; i++)
    {
        /**创建一个新的 Detection 对象用于保存单个检测结果 */
        Detection det;

        /**设置检测对象的类别名称 */
        det.className = det_grp.results[i].name;

        /**
         * 构造边界框 cv::Rect：
         * 参数分别是 x, y, width, height 
         * */
        det.box = cv::Rect(det_grp.results[i].box.left,
                           det_grp.results[i].box.top,
                           det_grp.results[i].box.right - det_grp.results[i].box.left,
                           det_grp.results[i].box.bottom - det_grp.results[i].box.top);

        /**设置检测置信度（probability）*/
        det.confidence = det_grp.results[i].prop;

        /**设置类别 ID（这里暂时设为 0，如需使用可从 det_grp.results[i] 中提取） */
        det.class_id = 0;
        
        /**为当前检测框生成一个随机颜色，用于绘制时区分不同对象 */
        det.color = cv::Scalar(rand() % 255, rand() % 255, rand() % 255);

        /**将当前检测结果加入输出列表 */
        objects.push_back(det);
    }
}

// 加载模型，获取输入输出属性
nn_error_e Yolov5::LoadModel(const char *model_path)
{
    auto ret = engine_->LoadModelFile(model_path);
    if (ret != NN_SUCCESS)
    {
        NN_LOG_ERROR("yolo load model file failed");
        return ret;
    }
    // get input tensor
    auto input_shapes = engine_->GetInputShapes();

    // check number of input and n_dims
    if (input_shapes.size() != 1)
    {
        NN_LOG_ERROR("yolo input tensor number is not 1, but %ld", input_shapes.size());
        return NN_RKNN_INPUT_ATTR_ERROR;
    }
    nn_tensor_attr_to_cvimg_input_data(input_shapes[0], input_tensor_);
    input_tensor_.data = malloc(input_tensor_.attr.size);

    auto output_shapes = engine_->GetOutputShapes();

    for (int i = 0; i < output_shapes.size(); i++)
    {
        tensor_data_s tensor;
        tensor.attr.n_elems = output_shapes[i].n_elems;
        tensor.attr.n_dims = output_shapes[i].n_dims;
        for (int j = 0; j < output_shapes[i].n_dims; j++)
        {
            tensor.attr.dims[j] = output_shapes[i].dims[j];
        }
        // output tensor needs to be float32
        if (output_shapes[i].type != NN_TENSOR_INT8)
        {
            NN_LOG_ERROR("yolo output tensor type is not int8, but %d", output_shapes[i].type);
            return NN_RKNN_OUTPUT_ATTR_ERROR;
        }
        tensor.attr.type = output_shapes[i].type;
        tensor.attr.index = i;
        tensor.attr.size = output_shapes[i].n_elems * nn_tensor_type_to_size(tensor.attr.type);
        tensor.data = malloc(tensor.attr.size);
        output_tensors_.push_back(tensor);
        out_zps_.push_back(output_shapes[i].zp);
        out_scales_.push_back(output_shapes[i].scale);
    }
    return NN_SUCCESS;
}

nn_error_e Yolov5::Preprocess(const cv::Mat &image)
{
    /**将预处理后的结果放入input_tensor_中 */
    cvimg2tensor(image, input_tensor_.attr.dims[2], input_tensor_.attr.dims[1], input_tensor_);
    return NN_SUCCESS;
}

nn_error_e Yolov5::Inference()
{
    std::vector<tensor_data_s> inputs;
    // 将input_tensor_放入inputs中
    inputs.push_back(input_tensor_);
    // 运行模型
    engine_->Run(inputs, output_tensors_, false);
    return NN_SUCCESS;
}

nn_error_e Yolov5::Postprocess(const cv::Mat &img, std::vector<Detection> &objects)
{
    int height = input_tensor_.attr.dims[1];
    int width = input_tensor_.attr.dims[2];
    float scale_w = height * 1.f / img.cols;
    float scale_h = width * 1.f / img.rows;

    yolov5::detect_result_group_t detections;

    yolov5::post_process((int8_t *)output_tensors_[0].data,
                         (int8_t *)output_tensors_[1].data,
                         (int8_t *)output_tensors_[2].data,
                         height, width,
                         BOX_THRESH, NMS_THRESH,
                         scale_w, scale_h,
                         out_zps_, out_scales_,
                         &detections);

    DetectionGrp2DetectionArray(detections, objects);
    return NN_SUCCESS;
}

nn_error_e Yolov5::Run(const cv::Mat &img, std::vector<Detection> &objects)
{
    Preprocess(img);           // 图像预处理
    Inference();               // 推理
    Postprocess(img, objects); // 后处理
    return NN_SUCCESS;
}

/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-05-08, lium
* describe: 初始创建.
*************************************************************************/