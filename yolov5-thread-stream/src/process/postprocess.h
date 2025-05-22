#ifndef POSTPROCESS_H_
#define POSTPROCESS_H_

#include <stdint.h>
#include <vector>
#include <string>

#define OBJ_NAME_MAX_SIZE 16
#define OBJ_NUMB_MAX_SIZE 64

namespace yolov5 {

    typedef struct _BOX_RECT {
        int left;
        int right;
        int top;
        int bottom;
    } BOX_RECT;

    typedef struct __detect_result_t {
        char name[OBJ_NAME_MAX_SIZE];
        BOX_RECT box;
        int id;
        float prop;
    } detect_result_t;

    typedef struct _detect_result_group_t {
        int id;
        int count;
        detect_result_t results[OBJ_NUMB_MAX_SIZE];
    } detect_result_group_t;

    int post_process(int8_t *input0, int8_t *input1, int8_t *input2, int model_in_h, int model_in_w,
                     float conf_threshold, float nms_threshold, float scale_w, float scale_h,
                     std::vector<int32_t> &qnt_zps, std::vector<float> &qnt_scales,
                     detect_result_group_t *group,std::string model_labels_path,int class_num);

    static float NMS_threshold;
    static float box_threshold;
    static std::string model_label_file_path;
    static int obj_class_num;
}
#endif
