#include <iostream>
#include <opencv2/opencv.hpp>
#include "rknn_api.h" // 确保包含正确的头文件

std::string imgPath = "images/bus.jpg";

// 加载模型文件的函数
unsigned char* load_model(const char* filename, int* model_size) {
    FILE* fp = fopen(filename, "rb");
    if (fp == nullptr) return nullptr;

    fseek(fp, 0, SEEK_END);
    *model_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    unsigned char* data = new unsigned char[*model_size];
    fread(data, *model_size, 1, fp);
    fclose(fp);

    return data;
}

// 打印张量属性的函数
void dump_tensor_attr(rknn_tensor_attr* attr) {
    printf("index=%d name=%s n_dims=%d dims=[%d %d %d %d] n_elems=%d size=%d fmt=%d type=%d qnt_type=%d fl=%f zp=%d\n",
           attr->index, attr->name, attr->n_dims, attr->dims[0], attr->dims[1], attr->dims[2], attr->dims[3],
           attr->n_elems, attr->size, attr->fmt, attr->type, attr->qnt_type, attr->fl, attr->zp);
}

// 中心裁剪函数
cv::Mat centerCrop(const cv::Mat& img, const cv::Size& crop_size) {
    int w = img.cols;
    int h = img.rows;
    int x = (w - crop_size.width) / 2;
    int y = (h - crop_size.height) / 2;
    return img(cv::Rect(x, y, crop_size.width, crop_size.height));
}

// 图像转换函数，这里仅作示意
cv::Mat transfer(const cv::Mat& img) {
    cv::Mat resized_img;
    cv::resize(img, resized_img, cv::Size(224, 224)); // 调整大小为224x224作为示例
    return resized_img;
}

// Softmax函数
std::vector<float> softmax(const std::vector<float>& input) {
    float max_val = *std::max_element(input.begin(), input.end());
    std::vector<float> exp_values(input.size());
    float sum_exp = 0.0f;
    for (size_t i = 0; i < input.size(); ++i) {
        exp_values[i] = exp(input[i] - max_val);
        sum_exp += exp_values[i];
    }
    for (auto& val : exp_values) val /= sum_exp;
    return exp_values;
}

int main(int argc, char **argv)
{
    const char *model_file = "weigths/yolov5s.rknn";
    const char *img_file = "images/bus.jpg";
 
    rknn_context ctx;
 
    int model_len = 0;                               // 模型文件大小
    auto model = load_model(model_file, &model_len); // 加载模型文件
    if (model == nullptr)
    {
        printf("load model file %s fail!", model_file);
        return -1; 
    }
 
    // 初始化rknn模型
    int ret = rknn_init(&ctx, model, model_len, 0, NULL); // 初始化rknn context
    if (ret < 0)
    {
        printf("rknn_init fail! ret=%d", ret);
        return -1; 
    }
 
    // 获取rknn版本信息 
    rknn_sdk_version version;
    ret = rknn_query(ctx, RKNN_QUERY_SDK_VERSION, &version, sizeof(rknn_sdk_version));
    if (ret < 0)
    {
        printf("rknn_init error ret=%d\n", ret);
        return -1;
    }
    printf("sdk version: %s driver version: %s\n", version.api_version, version.drv_version);
 
    // 获取rknn输入输出个数
    rknn_input_output_num io_num;
    ret = rknn_query(ctx, RKNN_QUERY_IN_OUT_NUM, &io_num, sizeof(io_num));
    if (ret < 0)
    {
        printf("rknn_init error ret=%d\n", ret);
        return -1;
    }
    printf("model input num: %d, output num: %d\n", io_num.n_input, io_num.n_output);
 
    // 获取rknn输入属性
    rknn_tensor_attr input_attrs[io_num.n_input];
    memset(input_attrs, 0, sizeof(input_attrs));
    for (int i = 0; i < io_num.n_input; i++)
    {
        input_attrs[i].index = i;
        ret = rknn_query(ctx, RKNN_QUERY_INPUT_ATTR, &(input_attrs[i]), sizeof(rknn_tensor_attr));
        if (ret < 0)
        {
        printf("rknn_init error ret=%d\n", ret);
        return -1;
        }
        dump_tensor_attr(&(input_attrs[i]));
    }
 
    // 获取rknn输出属性
    rknn_tensor_attr output_attrs[io_num.n_output];
    memset(output_attrs, 0, sizeof(output_attrs));
    for (int i = 0; i < io_num.n_output; i++)
    {
        output_attrs[i].index = i;
        ret = rknn_query(ctx, RKNN_QUERY_OUTPUT_ATTR, &(output_attrs[i]), sizeof(rknn_tensor_attr));
        dump_tensor_attr(&(output_attrs[i]));
    }
 
    int channel = 3;
    int width = 0;
    int height = 0;
    if (input_attrs[0].fmt == RKNN_TENSOR_NCHW)
    {
        printf("model is NCHW input fmt\n");
        channel = input_attrs[0].dims[1];
        height = input_attrs[0].dims[2];
        width = input_attrs[0].dims[3];
    }
    else
    {
        printf("model is NHWC input fmt\n");
        height = input_attrs[0].dims[1];
        width = input_attrs[0].dims[2];
        channel = input_attrs[0].dims[3];
    }
 
    printf("model input height=%d, width=%d, channel=%d\n", height, width, channel);
 
 
    // 读取图片
    cv::Mat orig_img = cv::imread(img_file, 1);
    if (!orig_img.data)
    {
        printf("cv::imread %s fail!\n", img_file);
        return -1;
    }
    cv::Size crop_size(224, 224);
 
    cv::Mat cropped_image = centerCrop(orig_img, crop_size);
 
    cv::Mat transferred_image = transfer(cropped_image);
    printf("transferred_image width = %d, transferred_image height = %d\n", transferred_image.cols, transferred_image.rows);
 
    // rknn_input inputs[1];
    // memset(inputs, 0, sizeof(inputs));
    // inputs[0].index = 0;
    // inputs[0].type = RKNN_TENSOR_FLOAT32;
    // inputs[0].size = transferred_image.total() * transferred_image.elemSize();
    // inputs[0].fmt = RKNN_TENSOR_NHWC;
    // inputs[0].pass_through = false; 
    // inputs[0].buf = transferred_image.data;
 
    // rknn_inputs_set(ctx, io_num.n_input, inputs);
 
    // rknn_output outputs[io_num.n_output];
    // memset(outputs, 0, sizeof(outputs));
    // for (int i = 0; i < io_num.n_output; i++)
    // {
    //     outputs[i].index = i;
    //     outputs[i].want_float = true;
    // } 
 
    // // 执行推理
    // ret = rknn_run(ctx, NULL);
    // ret = rknn_outputs_get(ctx, io_num.n_output, outputs, NULL);
 
    // size_t num_elements = outputs->size / 4; // 输出元素的数量
    // std::cout << outputs->size << std::endl;
    // std::vector<float> output_vector(num_elements);
    // std::memcpy(output_vector.data(), outputs->buf, outputs->size);
    // // 打印前几个元素作为示例
    // for (size_t i = 0; i < std::min(10ul, num_elements); ++i) {
    //     std::cout << "Output[" << i << "]: " << output_vector[i] << std::endl;
    // }
    // std::vector<float> probs = softmax(output_vector);
    return 0;  
}