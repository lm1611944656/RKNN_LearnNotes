/*-------------------------------------------
                Includes
-------------------------------------------*/
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <chrono>
#include <ctime>
#include <sys/time.h>
#include <sys/stat.h>
#include <opencv2/opencv.hpp>

#include "im2d.h"
#include "rga.h"
#include "RgaUtils.h"

#include "rknn_api.h"

#include "rkmedia/utils/mpp_decoder.h"
#include "rkmedia/utils/mpp_encoder.h"

#include "mk_mediakit.h"
#include "task/yolov5.h"
#include "draw/cv_draw.h"
#include "task/yolov5_thread_pool.h"

#include "reconfig/ReConfig.h"

typedef struct
{
    MppDecoder *decoder;
    MppEncoder *encoder;
    mk_media media;
    mk_pusher pusher;
    const char *push_url;
    uint64_t pts;
    uint64_t dts;
    // 记录一下我player不用重新初始化创建
    mk_player player;

    /**
     * 用来做预警视频保存的配套参数
     */
    // 记录最后一次录制时间
    std::chrono::system_clock::time_point warning_last_save_time = std::chrono::system_clock::now();
    // 读取配置参数的信息
    rr::RrConfig *config;

    // 记录一下自己的视频流地址
    const char *stream_url;
    // 记录一些配置参数
    int source_frame_rate;
    int source_width;
    int source_height;
    // 记录264还是265
    int source_video_type;


    int enable_rtsp;
    int push_rtsp_port;

    std::string push_path_first;
    std::string push_path_second;
   
    //zlmediakit配置文件路径
    std::string mk_file_path;

    // 用来计算FPS
    int frame_count = 0;
    std::chrono::_V2::system_clock::time_point start_all = std::chrono::high_resolution_clock::now();
    float out_fps = 0;

    // 播放失败统计
    int fail_count = 0;

} rknn_app_context_t;

void release_media(mk_media *ptr)
{
    if (ptr && *ptr)
    {
        mk_media_release(*ptr);
        *ptr = NULL;
    }
}

void release_pusher(mk_pusher *ptr)
{
    if (ptr && *ptr)
    {
        // 释放推流器
        mk_pusher_release(*ptr);
        *ptr = NULL;
    }
}

static Yolov5 *yolov5 = nullptr;                       // 模型
static Yolov5ThreadPool *yolov5_thread_pool = nullptr; // 线程池

// FOR WRITER线程安全的队列用于存储图像
std::queue<cv::Mat> image_queue;
std::mutex image_queue_mutex;
std::condition_variable image_queue_cv;
void writeToFileThread(void *userdata);

// 解码后的数据回调函数
void mpp_decoder_frame_callback(void *userdata, int width_stride, int height_stride, int width, int height, int format, int fd, void *data)
{

    rknn_app_context_t *ctx = (rknn_app_context_t *)userdata;

    int ret = 0;
    static int frame_index = 0;
    frame_index++;

    void *mpp_frame = NULL;
    int mpp_frame_fd = 0;
    void *mpp_frame_addr = NULL;
    int enc_data_size;

    rga_buffer_t origin;
    rga_buffer_t src;

    // 编码器准备
    if (ctx->encoder == NULL)
    {
        MppEncoder *mpp_encoder = new MppEncoder();
        MppEncoderParams enc_params;
        memset(&enc_params, 0, sizeof(MppEncoderParams));
        enc_params.width = width;
        enc_params.height = height;
        enc_params.hor_stride = width_stride;
        enc_params.ver_stride = height_stride;
        enc_params.fmt = MPP_FMT_YUV420SP;
        // 根据video_type判断264还是265
        if(ctx->source_video_type == 264)
            enc_params.type = MPP_VIDEO_CodingAVC;
        if(ctx->source_video_type == 265)
            enc_params.type = MPP_VIDEO_CodingHEVC;
        mpp_encoder->Init(enc_params, NULL);
        ctx->encoder = mpp_encoder;

        ctx->source_width = width;
        ctx->source_height = height;

    }

    int enc_buf_size = ctx->encoder->GetFrameSize();
    char *enc_data = (char *)malloc(enc_buf_size);

    mpp_frame = ctx->encoder->GetInputFrameBuffer();
    mpp_frame_fd = ctx->encoder->GetInputFrameBufferFd(mpp_frame);
    mpp_frame_addr = ctx->encoder->GetInputFrameBufferAddr(mpp_frame);

    // 复制到另一个缓冲区，避免修改mpp解码器缓冲区
    // 使用的是RK RGA的格式转换：YUV420SP -> RGB888
    origin = wrapbuffer_fd(fd, width, height, RK_FORMAT_YCbCr_420_SP, width_stride, height_stride);
    src = wrapbuffer_fd(mpp_frame_fd, width, height, RK_FORMAT_YCbCr_420_SP, width_stride, height_stride);
    cv::Mat origin_mat = cv::Mat::zeros(height, width, CV_8UC3);
    rga_buffer_t rgb_img = wrapbuffer_virtualaddr((void *)origin_mat.data, width, height, RK_FORMAT_RGB_888);
    imcopy(origin, rgb_img);

    // 推送到推理队列的任务数量
    static int job_cnt = 0;
    // 获取到结果的任务数量，如果获取失败就可能job_cnt和result_cnt不一致
    static int result_cnt = 0;

    // 提交推理任务给线程池
    yolov5_thread_pool->submitTask(origin_mat, job_cnt++);

    std::vector<Detection> objects;
    // 获取推理结果
    auto ret_code = yolov5_thread_pool->getTargetResultNonBlock(objects, result_cnt);

    // 计算FPS
    // 结束计时
    auto end_time = std::chrono::high_resolution_clock::now();

    // 将当前时间点转换为毫秒级别的时间戳
    auto millis = std::chrono::time_point_cast<std::chrono::milliseconds>(end_time).time_since_epoch().count();

    ctx->frame_count++;
    auto elapsed_all_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - ctx->start_all).count() / 1000.f;
    // 每隔1秒打印一次
    if (elapsed_all_time > 1000)
    {
        ctx->out_fps = (ctx->frame_count / (elapsed_all_time / 1000.0f));
        // LOG
        NN_LOG_INFO("=================================>>>>>>>>>>>>>>>>>> Time:%fms, FPS:%f, Frame Count:%d\n", elapsed_all_time, ctx->out_fps, ctx->frame_count);
        ctx->frame_count = 0;
        ctx->start_all = std::chrono::high_resolution_clock::now();
    }

    if (ret_code == NN_SUCCESS)
    {
        result_cnt++;
    }
    else
    {
        // 千万要这么写不然会内存溢出让系统奔溃
        goto RET;
        
    }
    // 在img上画出检测结果
    DrawDetections(origin_mat, objects);
    // rgb_img>>src
    imcopy(rgb_img, src);
    
    // 编码
    memset(enc_data, 0, enc_buf_size);
    enc_data_size = ctx->encoder->Encode(mpp_frame, enc_data, enc_buf_size);
    ret = mk_media_input_h264(ctx->media, enc_data, enc_data_size, millis, millis);

    if (ret != 1)
    {
        printf("mk_media_input_frame failed\n");
    }

RET: // tag
    if (enc_data != nullptr)
    {
        free(enc_data);
    }
}


/**
 * track数据回调
*/
void API_CALL on_track_frame_out(void *user_data, mk_frame frame)
{
    rknn_app_context_t *ctx = (rknn_app_context_t *)user_data;
    // 打印ctx中的dts
    // printf("on_track_frame_out ctx->dts=%llu ctx->pts=%llu  \n", ctx->dts, ctx->pts);
    const char *data = mk_frame_get_data(frame);
    ctx->dts = mk_frame_get_dts(frame);
    ctx->pts = mk_frame_get_pts(frame);
    // printf(">>>>>>>>>>>>>>>>>>>>>>>ctx->dts=%llu===============ctx->pts=%llu\n",ctx->dts,ctx->pts);
    size_t size = mk_frame_get_data_size(frame);
    // 
    auto code_name = mk_frame_codec_name(frame);
    // LOG
    // printf("on_track_frame_out codec_name=%s codec_id=%d  \n",code_name,mk_frame_codec_id(frame));
    // 如果code_name是H264或者H265
    // 这段日志太多了
    // printf("decoder=%p\n", ctx->decoder); 
    ctx->decoder->Decode((uint8_t *)data, size, 0);
    // mk_media_input_frame(ctx->media, frame);

        
}

/**
 * 推流事件回调
*/
void API_CALL on_mk_push_event_func(void *user_data, int err_code, const char *err_msg)
{
    rknn_app_context_t *ctx = (rknn_app_context_t *)user_data;
    if (err_code == 0)
    {
        // push success
        log_info("push %s success!", ctx->push_url);
        printf("push %s success!\n", ctx->push_url);
    }
    else
    {
        log_warn("push %s failed:%d %s", ctx->push_url, err_code, err_msg);
        printf("push %s failed:%d %s\n", ctx->push_url, err_code, err_msg);
        release_pusher(&(ctx->pusher));
    }
}

/**
 * MediaSource注册或注销事件回调函数
*/
void API_CALL on_mk_media_source_regist_func(void *user_data, mk_media_source sender, int regist)
{
    // printf("mk_media_source:%x\n", sender);
    rknn_app_context_t *ctx = (rknn_app_context_t *)user_data;
    const char *schema = mk_media_source_get_schema(sender);
    if (strncmp(schema, ctx->push_url, strlen(schema)) == 0)
    {
        // 判断是否为推流协议相关的流注册或注销事件
        printf("schema: %s\n", schema);
        release_pusher(&(ctx->pusher));
        if (regist)
        {
            // 绑定的MediaSource对象并创建rtmp[s]/rtsp[s]推流器 MediaSource通过mk_media_create或mk_proxy_player_create或推流生成 该MediaSource对象必须已注册
            ctx->pusher = mk_pusher_create_src(sender);
            // 设置推流器推流结果回调函数
            mk_pusher_set_on_result(ctx->pusher, on_mk_push_event_func, ctx);
            // 设置推流被异常中断的回调
            mk_pusher_set_on_shutdown(ctx->pusher, on_mk_push_event_func, ctx);
            //            mk_pusher_publish(ctx->pusher, ctx->push_url);
            log_info("push started!");
            printf("push started!\n");
        }
        else
        {
            log_info("push stoped!");
            printf("push stoped!\n");
        }
        printf("push_url:%s\n", ctx->push_url);
    }
    else
    {
        printf("unknown schema:%s\n", schema);
    }
}

/**
 * 播放器事件回调
*/
void API_CALL on_mk_play_event_func(void *user_data, int err_code, const char *err_msg, mk_track tracks[],
                                    int track_count)
{
    rknn_app_context_t *ctx = (rknn_app_context_t *)user_data;
    if (err_code == 0)
    {
        // success
        printf("play success!");
        //终止异常重连次数
        ctx->fail_count=0;
        // 用在重新连接的时候初始化一些media和pusher不然会存在对象报错
        release_media(&(ctx->media));
        release_pusher(&(ctx->pusher));
        int i;
        ctx->push_url = "rtsp://localhost";
        // 创建MediaSource,参数5开启hls，可以用http://146.56.190.132:13180/live/test/hls.fmp4.m3u8访问
        ctx->media = mk_media_create("__defaultVhost__", ctx->push_path_first.c_str(), ctx->push_path_second.c_str(), 0, 0, 0);
        for (i = 0; i < track_count; ++i)
        {
            if (mk_track_is_video(tracks[i]))
            {
                log_info("got video track: %s", mk_track_codec_name(tracks[i]));
                // 监听track数据回调
                mk_media_init_track(ctx->media, tracks[i]);
                mk_track_add_delegate(tracks[i], on_track_frame_out, user_data);
            }else{
                log_info("got audio track: %s", mk_track_codec_name(tracks[i]));
                // 监听track数据回调
                mk_media_init_track(ctx->media, tracks[i]);
                mk_track_add_delegate(tracks[i], on_track_frame_out, user_data);
            }
        }
        // 初始化h264/h265/aac完毕后调用此函数， 在单track(只有音频或视频)时，因为ZLMediaKit不知道后续是否还要添加track，
        // 所以会多等待3秒钟 如果产生的流是单Track类型，请调用此函数以便加快流生成速度，当然不调用该函数，影响也不大(会多等待3秒)
        // TEST
        mk_media_init_complete(ctx->media);
        // 设置MediaSource注册或注销事件回调函数
        mk_media_set_on_regist(ctx->media, on_mk_media_source_regist_func, ctx);
        //      codec_args v_args = {0};
        //      mk_track v_track = mk_track_create(MKCodecH264, &v_args);
        //      mk_media_init_track(ctx->media, v_track);
        //      mk_media_init_complete(ctx->media);
        //      mk_track_unref(v_track);
    }
    else
    {   
        // 如果播放重练超过10次，就让系统自动结束，等待保护进程重新启动
        if(ctx->fail_count++ < 10){
            printf("play failed: %d %s", err_code, err_msg);
            printf("===============play try again start!for play===============\n");
            rknn_app_context_t *ctx = (rknn_app_context_t *)user_data;
            mk_player_play(ctx->player, ctx->stream_url);
            sleep(5);
            printf("===============play try again end!for play===============\n");
        }else{
            printf("play failed: %d %s", err_code, err_msg);
            printf("===============play shutdown!!!!!!===============\n");
        }
    }
}

/**
 * 这是播放器的异常事件回调函数。这个函数完全可以和on_mk_play_event_func合并。
 * 保留的内容和on_mk_play_event_func中的else部分一致。
 *
 *
 *
 * */
void API_CALL on_mk_shutdown_func(void *user_data, int err_code, const char *err_msg, mk_track tracks[], int track_count)
{
    printf("play interrupted: %d %s", err_code, err_msg);
    printf("===============play try again start!for shutdown===============\n");
    rknn_app_context_t *ctx = (rknn_app_context_t *)user_data;
    mk_player_play(ctx->player, ctx->stream_url);
    sleep(5);
    printf("===============play try again end!for shutdown===============\n");
}

/**
 * 处理来自指定 RTSP URL 的视频。
 *
 * @param ctx 指向 RKNN 应用上下文的指针。
 * @param url 要处理的 RTSP URL。
 *
 * @return 函数的退出代码。
 *
 * @throws 无。
 */
int process_video_rtsp(rknn_app_context_t *ctx, const char *url)
{
    // mk的参数初始化
    char *ini_path = mk_util_get_exe_dir(ctx->mk_file_path.c_str());
    mk_config config = {
            .thread_num = 10,
            .log_level = 0,
            .log_mask = LOG_CONSOLE,
            .log_file_path = NULL,
            .log_file_days = 0,
            .ini_is_path = 1,
            .ini = ini_path,
            .ssl_is_path = 1,
            .ssl = NULL,
            .ssl_pwd = NULL
    };
    
    // memset(&config, 0, sizeof(mk_config));
    // config.log_mask = LOG_CONSOLE;
    // 初始化环境，调用该库前需要先调用此函数
    mk_env_init(&config);
    mk_free(ini_path);


    // 在端口554上启动RTSP服务器
    if (ctx->enable_rtsp == 1)
        mk_rtsp_server_start(ctx->push_rtsp_port, 0);

    // 创建一个新的Codeium播放器
    ctx->player = mk_player_create();
    ctx->stream_url = url;
    // 设置处理播放事件的回调函数
    mk_player_set_on_result(ctx->player, on_mk_play_event_func, ctx);
    // 设置播放被异常中断的回调
    mk_player_set_on_shutdown(ctx->player, on_mk_shutdown_func, ctx);
    // 播放来自提供的RTSP URL的视频流
    mk_player_play(ctx->player, ctx->stream_url);

    // printf("enter any key to exit\n");
    // getchar();
    //如果ctx->fail_count==10跳出循环
    printf("===============启动step1===============\n");
    while (1){
        if(ctx->fail_count>=10){
            break;
            printf("===============销毁===============\n");
        }
        sleep(10);
        printf("===============alive===============\n");
    }
    printf("===============启动step2===============\n");    
    if (ctx->player)
    {
        mk_player_release(ctx->player);
    }
    return 0;
}

int main(int argc, char **argv)
{
    int status = 0;
    int ret;
    rknn_app_context_t app_ctx;                      // 创建上下文
    memset(&app_ctx, 0, sizeof(rknn_app_context_t)); // 初始化上下文

    if (argc != 2)
    {
        printf("Usage: %s <ini_path> \n", argv[0]);
        return -1;
    }

    rr::RrConfig config;
    bool config_ret = config.ReadConfig(argv[1]);
    if (config_ret == false)
    {
        printf("ReadConfig is Error,Cfg=%s", "yunyan_config.ini");
        return -1;
    }
    // 获取参数中的模型位置
    std::string model_path = config.ReadString("YUNYAN", "ModelPath", "");
    if (model_path == "")
    {
        printf("ModelPath not found!\n");
        return -1;
    }
    // NMSIOU
    float NMS_threshold = config.ReadFloat("YUNYAN", "NMSThreshold", 0.45);
    // 置信度
    float box_threshold = config.ReadFloat("YUNYAN", "BoxThreshold", 0.4);
    // 获取参数中的模型标签
    std::string model_label_file_path = config.ReadString("YUNYAN", "ModelLabelsFilePath", "");
    if (model_label_file_path == "")
    {
        printf("ModelLabelsFilePath not found!\n");
        return -1;
    }
    int obj_class_num = config.ReadInt("YUNYAN", "ObjClassNum", 80);
    // 获取参数中的视频流地址
    std::string stream_url_str = config.ReadString("YUNYAN", "StreamUrl", "");
    if (stream_url_str == "")
    {
        printf("StreamUrl not found!\n");
        return -1;
    }
    // 将string格式转换成char *
    char *stream_url = new char[strlen(stream_url_str.c_str()) + 1];
    strcpy(stream_url, stream_url_str.c_str());
    // 获取参数中的视频流类型
    int video_type = config.ReadInt("YUNYAN", "VideoType", 264);
    app_ctx.source_video_type = video_type;
    // 获取识别线程数量
    int num_threads = config.ReadInt("YUNYAN", "NumThreads", 12);
    app_ctx.source_frame_rate = config.ReadInt("YUNYAN", "SourceFrameRate", 25);
    app_ctx.enable_rtsp = config.ReadInt("YUNYAN", "EnableRtsp", 0);
    app_ctx.push_rtsp_port = config.ReadInt("YUNYAN", "PushRtspPort", 554);
    std::string push_pash_first_str = config.ReadString("YUNYAN", "PushPathFirst", "yunyan-live");
    app_ctx.push_path_first = push_pash_first_str;
    std::string push_pash_second_str = config.ReadString("YUNYAN", "PushPathSecond", "test");
    app_ctx.push_path_second = push_pash_second_str;

    std::string mk_file_path_str = config.ReadString("YUNYAN", "MkFilePath", "mk_config.ini");
    app_ctx.mk_file_path = mk_file_path_str;
    

    app_ctx.config = &config;

    printf("===============配置文件读取完毕===============\n");

    yolov5_thread_pool = new Yolov5ThreadPool();                                                                            // 创建线程池
    yolov5_thread_pool->setUp(model_path, num_threads, NMS_threshold, box_threshold, model_label_file_path, obj_class_num); // 初始化线程池

    // MPP 解码器
    if (app_ctx.decoder == NULL)
    {
        MppDecoder *decoder = new MppDecoder();  // 创建解码器
        decoder->Init(video_type, app_ctx.source_frame_rate, &app_ctx); // 初始化解码器
        // mpp在每次解析后都会回调mpp_decoder_frame_callback方法
        decoder->SetCallback(mpp_decoder_frame_callback); // 设置回调函数，用来处理解码后的数据
        app_ctx.decoder = decoder;                        // 将解码器赋值给上下文
    }

    printf("app_ctx=%p decoder=%p\n", &app_ctx, app_ctx.decoder);
    // 读取视频流，同时把参数传递器ctx一起传递，需要注意这里面就有解码器的对象
    process_video_rtsp(&app_ctx, stream_url);

    printf("waiting finish\n");
    usleep(3 * 1000 * 1000);


    if (app_ctx.decoder != nullptr)
    {
        delete (app_ctx.decoder);
        app_ctx.decoder = nullptr;
    }
    if (app_ctx.encoder != nullptr)
    {
        delete (app_ctx.encoder);
        app_ctx.encoder = nullptr;
    }

    return 0;
}