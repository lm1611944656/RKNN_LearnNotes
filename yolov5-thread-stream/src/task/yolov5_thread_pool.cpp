
#include "yolov5_thread_pool.h"
#include "draw/cv_draw.h"

void Yolov5ThreadPool::worker(int id)
{
    while (!stop)
    {
        std::pair<int, cv::Mat> task;
        std::shared_ptr<Yolov5> instance = yolov5_instances[id];
        {
            std::unique_lock<std::mutex> lock(mtx1);
            cv_task.wait(lock, [&]
                         { return !tasks.empty() || stop; });

            if (stop)
            {
                return;
            }

            task = tasks.front();
            tasks.pop();
        }

        std::vector<Detection> detections;
        instance->Run(task.second, detections);

        {
            std::lock_guard<std::mutex> lock(mtx2);
            results.insert({task.first, detections});
            DrawDetections(task.second, detections);
            img_results.insert({task.first, task.second});
            cv_result.notify_one();
        }
    }
}

nn_error_e Yolov5ThreadPool::setUp(std::string &model_path, int num_threads, float NMS_threshold, float box_threshold,
                                   std::string model_label_file_path, int obj_class_num)
{
    for (size_t i = 0; i < num_threads; ++i)
    {
        // 创建模型
        std::shared_ptr<Yolov5> yolov5 = std::make_shared<Yolov5>();
        yolov5->LoadModel(model_path.c_str());
        yolov5->setStaticParams(NMS_threshold, box_threshold, model_label_file_path, obj_class_num);
        yolov5_instances.push_back(yolov5);
    }
    for (size_t i = 0; i < num_threads; ++i)
    {
        threads.emplace_back(&Yolov5ThreadPool::worker, this, i);
    }
    return NN_SUCCESS;
}

Yolov5ThreadPool::Yolov5ThreadPool() { stop = false; }

Yolov5ThreadPool::~Yolov5ThreadPool()
{
    stop = true;
    cv_task.notify_all();
    for (auto &thread : threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
}

nn_error_e Yolov5ThreadPool::submitTask(const cv::Mat &img, int id)
{
    {
        std::lock_guard<std::mutex> lock(mtx1);
        tasks.push({id, img});
    }
    cv_task.notify_one();
    return NN_SUCCESS;
}

nn_error_e Yolov5ThreadPool::getTargetResult(std::vector<Detection> &objects, int id)
{
    int loop_cnt = 0;
    while (results.find(id) == results.end())
    {
        // sleep 1ms
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        loop_cnt++;
        if (loop_cnt > 1000)
        {
            NN_LOG_WARNING("getTargetImgResult timeout");
            return NN_TIMEOUT;
        }
    }
    std::lock_guard<std::mutex> lock(mtx2);
    objects = results[id];
    // remove from map
    results.erase(id);
    img_results.erase(id);
    return NN_SUCCESS;
}

// 非阻塞获取结果
nn_error_e Yolov5ThreadPool::getTargetResultNonBlock(std::vector<Detection> &objects, int id)
{
    if (results.find(id) == results.end())
    {
        return NN_RESULT_NOT_READY;
    }
    std::lock_guard<std::mutex> lock(mtx2);
    objects = results[id];
    // remove from map
    results.erase(id);
    img_results.erase(id);
    return NN_SUCCESS;
}

nn_error_e Yolov5ThreadPool::getTargetImgResult(cv::Mat &img, int id)
{
    int loop_cnt = 0;
    while (img_results.find(id) == img_results.end())
    {
        // sleep 1ms
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        loop_cnt++;
        if (loop_cnt > 1000)
        {
            NN_LOG_WARNING("getTargetImgResult timeout");
            return NN_TIMEOUT;
        }
    }
    std::lock_guard<std::mutex> lock(mtx2);
    img = img_results[id];
    // remove from map
    img_results.erase(id);
    results.erase(id);

    return NN_SUCCESS;
}

// 停止所有线程
void Yolov5ThreadPool::stopAll()
{
    stop = true;
    cv_task.notify_all();
}
