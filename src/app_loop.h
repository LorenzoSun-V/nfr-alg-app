/*
* @file:   app_loop.h
* @Author: leo
* @Mail:   jzyleomessi@gmail.com
* @Date:   2024-11-25
* @brief:  该文件定义了程序的主循环，包括相机获取、推流、推断等功能
*/
#include <iostream>
#include <string>
#include "app.h"
#include <chrono>
#include "app_video.h"
#include "app_yolov8.h"
#include "app_yolov8obb.h"
// #include "app_facefeature.h"
// #include "app_retinaface.h"
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include "app_timer.h"
#include "app_init.h"

using json = nlohmann::json;

// 获取系统时间
std::string formatTimestamp(NowTime& time) {
    // 获取当前时间点
    auto now = std::chrono::system_clock::now();
    
    // 转换为time_t
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    
    // 获取本地时间
    std::tm* localTime = std::localtime(&currentTime);
    
    // 获取毫秒
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    // 存入NowTime结构体
    time.year = localTime->tm_year + 1900;
    time.mon = localTime->tm_mon + 1;
    time.day = localTime->tm_mday;
    time.hour = localTime->tm_hour;
    time.min = localTime->tm_min;
    time.sec = localTime->tm_sec;
    time.msec = milliseconds.count();

    // 格式化时间字符串
    char timeStr[64] = {0};
    std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localTime);
    std::string strTime(timeStr);

    // 格式化毫秒字符串
    char msecStr[16] = {0};
    std::snprintf(msecStr, sizeof(msecStr), ".%d", time.msec);
    std::string strMsec(msecStr);

    // 拼接时间字符串和毫秒字符串
    std::string strTimestamp = strTime + strMsec;

    return strTimestamp;
}

// 将 cv::Rect 转换为 std::vector<cv::Point>
std::vector<cv::Point> rectToPoints(const cv::Rect& rect) {
    std::vector<cv::Point> points;
    points.push_back(rect.tl()); // 左上角
    points.push_back(cv::Point(rect.br().x, rect.tl().y)); // 右上角
    points.push_back(rect.br()); // 右下角
    points.push_back(cv::Point(rect.tl().x, rect.br().y)); // 左下角
    return points;
}

double calculateIoU( const std::vector<cv::Point>& polygon, const std::vector<cv::Point>& quad) {
    // 验证输入的四边形确实是四边形
    if (quad.size() != 4) {
        throw std::invalid_argument("First argument must be a quadrilateral (4 points)");
    }
    
    // 验证多边形至少有3个点
    if (polygon.size() < 3) {
        throw std::invalid_argument("Second argument must be a polygon with at least 3 points");
    }

    // 计算多边形的交集
    std::vector<cv::Point> intersectionPolygon;
    float intersectionArea = cv::intersectConvexConvex(quad, polygon, intersectionPolygon);

    // 计算两个多边形的面积
    double quadArea = cv::contourArea(quad);
    double polygonArea = cv::contourArea(polygon);

    // 计算并集面积
    double unionArea = quadArea + polygonArea - intersectionArea;

    // 防止除以0
    if (unionArea <= 0) {
        return 0.0;
    }

    // 计算IoU
    double iou = intersectionArea / unionArea;

    // 确保结果在[0,1]范围内
    return std::max(0.0, std::min(1.0, iou));
}


// 重置相机
// void ReconnectCamera(std::unique_ptr<AppVideo> &rtspCamera, const RtspServerParam &rtspcfg){
//     std::this_thread::sleep_for(std::chrono::seconds(3));
//     bool ret_destory = rtspCamera->DestoryRtspDecoder();            
//     bool ret_init = rtspCamera->CreateRtspDecoder(rtspcfg.url,rtspcfg.dtype);
// }
bool ReconnectCamera(std::unique_ptr<AppVideo> &rtspCamera, const RtspServerParam &rtspcfg) {
    std::this_thread::sleep_for(std::chrono::seconds(3));
    rtspCamera.reset(new AppVideo());
    if (!rtspCamera->CreateRtspDecoder(rtspcfg.url, rtspcfg.dtype)) {
        std::cerr << "ReconnectCamera: CreateRtspDecoder failed" << std::endl;
        return false;
    }
    return true;
}


// 生产者线程函数
void RtspProducerThread(SharedData& sharedData, const RtspServerParam& rtspParam, std::unique_ptr<AppVideo> appRtsp,GlobalParam& globalparam) {
    auto interval = std::chrono::milliseconds(1000 / globalparam.frame_rate);
    const int MAX_CONSECUTIVE_ERRORS = 5;       // 最大连续错误次数
    const int ERROR_RETRY_DELAY_MS = 100;       // 错误重试延迟（毫秒）
    const int RECONNECT_DELAY_MS = 1000;        // 重新连接延迟（毫秒）
    int errorCounter = 0;                       // 连续错误计数器

    if (rtspParam.switch_flag == 0) {
        std::cerr << "RtspProducerThread: RTSP stream " << rtspParam.id << " is disabled, exiting." << std::endl;
        return;
    }

    rtspframe frameData;
    frameData.frame = cv::Mat(2000, 2000, CV_8UC3);

    while (sharedData.isRunning) {
        
        NowTime time;
        bool ret = appRtsp->DecodeGetOneFrame(frameData.frame); // 从 RTSP 流中获取帧

        if (ret) {
            // 帧获取成功
            frameData.timestamp = formatTimestamp(time);
            frameData.rtsp_id = rtspParam.id;
            frameData.model_types = rtspParam.model_types;
            errorCounter = 0; // 重置错误计数器

            // 将帧放入所有相关类型的队列
            // for (const auto& type : frameData.model_types) {
            //     std::lock_guard<std::mutex> lock(sharedData.queueMutexes[type]);
            //     sharedData.frameQueues[type].push(frameData); // 将帧放入队列
            //     sharedData.ready = true; // 设置 ready 标志
            //     // std::cout << "Producer: Frame with types [" << type << "] from camera " << frameData.rtsp_id << " added to queue." << std::endl;
            //     sharedData.queueCVs[type].notify_one();
            // }
            for (const auto& type : frameData.model_types) {
                std::lock_guard<std::mutex> lock(sharedData.queueMutexes[type]);
                auto& queue = sharedData.frameQueues[type];

                if (queue.size() >= sharedData.MAX_QUEUE_SIZE) {
                    // 队列已满，丢弃最老的帧，确保队列里都是最新的帧
                    queue.pop();
                }
                queue.push(frameData);
                sharedData.ready = true;
                sharedData.queueCVs[type].notify_one();
            }
            // 帧获取间隔
            std::this_thread::sleep_for(interval);
        } 
        else {
            // 帧获取失败
            errorCounter++;
            std::cerr << "RtspProducerThread: Failed to get frame from RTSP stream " << rtspParam.id
                      << ", error count: " << errorCounter << std::endl;

            if (errorCounter > MAX_CONSECUTIVE_ERRORS) {
                // 长时间无法获取帧，尝试重新连接
                std::cerr << "RtspProducerThread: Max error count reached, attempting to reconnect RTSP stream "
                          << rtspParam.id << "..." << std::endl;

                try {
                    // ReconnectCamera(appRtsp, rtspParam); // 重新连接 RTSP 流
                    if (!ReconnectCamera(appRtsp, rtspParam)) {
                        // 如果重连失败，可以选择 break 退出线程，或者 sleep 重试
                        std::cerr << "RtspProducerThread: " << rtspParam.url << " reconnect failed, waiting before retry..." << std::endl;
                        std::this_thread::sleep_for(std::chrono::seconds(3));
                        continue; // 或 return;
                    }
                    errorCounter = 0;     // 重置错误计数器
                    std::cerr << "RtspProducerThread: Reconnected to RTSP stream " << rtspParam.id << std::endl;
                } catch (const std::exception& e) {
                    std::cerr << "RtspProducerThread: Reconnect failed for RTSP stream " << rtspParam.id
                              << ": " << e.what() << std::endl;
                }

                // 延迟一段时间后重试
                std::this_thread::sleep_for(std::chrono::milliseconds(RECONNECT_DELAY_MS));
            } else {
                // 延迟一段时间后重试
                std::this_thread::sleep_for(std::chrono::milliseconds(ERROR_RETRY_DELAY_MS));
            }
        }

        // 帧获取间隔
        // std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

// HBB检测处理函数
void HBBProcess(std::unique_ptr<AppYolov8> &appfiresmoke, const rtspframe &frameData, const RtspRegionParams& rtsp_regions, const GlobalParam& globalparam, std::map<std::string, json>& alarmMap) 
{
    cv::Mat frame = frameData.frame;
    // 检查输入数据
    if (frame.empty()) {
        std::cerr << "Frame is empty!" << std::endl;
        return;
    }
    // 检查指针是否为空
    if (!appfiresmoke) {
        std::cerr << "model firesmoke is nullptr!" << std::endl;
        return;
    }
    std::vector<DetBox> results;
    // 保存图片
    bool ret = appfiresmoke->RunModelInference(frame, results);
    if (ret) {
        for (auto& result : results) {
            json alarmInfo;
            alarmInfo["camera_id"] = frameData.rtsp_id;
            alarmInfo["timestamp"] = frameData.timestamp;
            
            // Convert result to a similar format
            std::vector<std::vector<int>> result_json = {
                {static_cast<int>(result.x), static_cast<int>(result.y)},
                {static_cast<int>(result.x + result.w), static_cast<int>(result.y)},
                {static_cast<int>(result.x + result.w), static_cast<int>(result.y + result.h)},
                {static_cast<int>(result.x), static_cast<int>(result.y + result.h)}
            };

            alarmInfo["box"] = result_json;

            int class_id = result.classID;
            SingalRtspRegionParam rtsp_region;
            for(auto& item: rtsp_regions.rtspRegionParams){
                if(class_id == item.id){
                    rtsp_region = item;
                }
            }

            std::string alarmType;
            
            //判断是否要计算iou
            // if(rtsp_region.switch_flag){
            //     double iou = calculateIoU(rtsp_region.points, rectToPoints(cv::Rect(result.x, result.y, result.w, result.h)));
            //     if(iou > rtsp_region.iou_thresh){
            //         alarmType = std::to_string(result.classID);
            //         //保存图片
            //         // cv::imwrite(std::to_string(result.classID) + std::to_string(frameData.rtsp_id) + frameData.timestamp +".jpg");
            //     }
            // }
            // else{
            //     alarmType = std::to_string(result.classID);
            // }

            if (!alarmType.empty()) {
                // 如果 alarmType 已存在，添加到对应的列表中；否则创建新列表
                if (alarmMap.find(alarmType) == alarmMap.end()) {
                    alarmMap[alarmType] = json::array();
                }
                alarmMap[alarmType].push_back(alarmInfo);
            }
        }   
    }
    if (results.size() > 0 && frameData.rtsp_id == 8){
        std::string model_name = appfiresmoke->GetModelName();
        std::string img_name = "rtsp_" + std::to_string(frameData.rtsp_id) + "_" + model_name + "_" + frameData.timestamp + ".jpg";
        cv::imwrite(img_name, frameData.frame);
    }
}

// OBB检测处理函数
void OBBProcess(std::unique_ptr<AppYolov8obb> &appYolov8OBB, const rtspframe &frameData, const RtspRegionParams& rtsp_regions ,const GlobalParam& globalparam ,std::map<std::string, json>& alarmMap) {
    cv::Mat frame = frameData.frame;
    // 检查输入数据
    if (frame.empty()) {
        std::cerr << "Frame is empty!" << std::endl;
        return;
    }
    // 检查指针是否为空
    if (!appYolov8OBB) {
        std::cerr << "appYolov8OBB is nullptr!" << std::endl;
        return;
    }
    std::vector<DetBox> results;
    // 保存图片
    bool ret = appYolov8OBB->RunModelInference(frame, results);
    if (ret) {
        for (auto& result : results) {
            json alarmInfo;
            alarmInfo["camera_id"] = frameData.rtsp_id;
            alarmInfo["timestamp"] = frameData.timestamp;
            std::vector<cv::Point> box_points = xywhr2xyxyxyxy(result);
            
            // Convert box_points to a JSON array
            std::vector<std::vector<int>> box_json;
            for (const auto& point : box_points) {
                // std::cout << point.x << " " << point.y << std::endl;
                box_json.push_back({point.x, point.y});
            }

            alarmInfo["box"] = box_json;

            std::string alarmType;
            
            int return_id = result.classID + globalparam.hbb_classnum;
            SingalRtspRegionParam rtsp_region;
            for(auto& item: rtsp_regions.rtspRegionParams){
                if(return_id == item.id){
                    rtsp_region = item;
                }
            }

            //判断是否要计算iou
            // if(rtsp_region.switch_flag){
            //     double iou = calculateIoU(rtsp_region.points, rectToPoints(cv::Rect(result.x, result.y, result.w, result.h)));
            //     if(iou > rtsp_region.iou_thresh){
            //         alarmType = std::to_string(result.classID);
            //         //保存图片
            //         // cv::imwrite("obstacle" + std::to_string(frameData.rtsp_id) + frameData.timestamp +".jpg");
            //     }
            // }
            // else{
            //     alarmType = std::to_string(result.classID);
            // }

            if (!alarmType.empty()) {
                // 如果 alarmType 已存在，添加到对应的列表中；否则创建新列表
                if (alarmMap.find(alarmType) == alarmMap.end()) {
                    alarmMap[alarmType] = json::array();
                }
                alarmMap[alarmType].push_back(alarmInfo);
            }
        }   
    }
    if (results.size() > 0 && frameData.rtsp_id == 8){
        std::string model_name = appYolov8OBB->GetModelName();
        std::string img_name = "rtsp_" + std::to_string(frameData.rtsp_id) + "_" + model_name + "_" + frameData.timestamp + ".jpg";
        cv::imwrite(img_name, frameData.frame);
    }
}

// HTTP 请求函数
void SendHttpRequest(const json& data, CURL* curl, const HTTPParam& httpcfg) {
    // CURL* curl = curl_easy_init();
    if (curl) {
        std::string jsonStr = data.dump();
        // std::cout << " send request: ..... "  << std::endl;
        // 设置 URL 和 POST 数据
        // std::cout << httpcfg.http_server_ip << std::endl;
        curl_easy_setopt(curl, CURLOPT_URL, httpcfg.http_server_ip.c_str());
        // curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8080");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonStr.c_str());

        // 设置 HTTP 头
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // 执行 HTTP 请求
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }

        // 清理
        curl_slist_free_all(headers);
        // curl_easy_cleanup(curl);
    }
}

// 消费者线程函数
void ConsumerThread(SharedData& sharedData, 
                    const std::string& modelType,
                    std::unique_ptr<AppYolov8> apphbb,
                    std::unique_ptr<AppYolov8obb> appobb,
                    const std::vector<RtspRegionParams>& regionParams,
                    const GlobalParam& globalParam,
                    const HTTPParam& httpcfg) {
    // std::cout << "ConsumerThread started for model type: " << modelType << std::endl;
    std::unordered_map<std::string, std::chrono::seconds> typeIntervalMap;
    for (const auto& [key, value] : globalParam.typeIntervalMap) {
        typeIntervalMap[key] = std::chrono::seconds(value);
    }


    auto lastSendTime = std::chrono::steady_clock::now();
    std::map<std::string, json> accumulatedAlarmMap;

    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize CURL" << std::endl;
        return;
    }
    while (sharedData.isRunning) {
        std::vector<rtspframe> frames;
        std::map<std::string, json> currentAlarmMap;  // 按 alarm_type 分类存储报警信息

        // 从帧队列中获取帧
        {
            std::unique_lock<std::mutex> lock(sharedData.queueMutexes[modelType]);
            sharedData.queueCVs[modelType].wait(lock, [&] { 
                return !sharedData.frameQueues[modelType].empty() || !sharedData.isRunning; 
            });

            if (!sharedData.isRunning) break;

            // 取出所有帧
            while (!sharedData.frameQueues[modelType].empty()) {
                frames.push_back(sharedData.frameQueues[modelType].front());
                sharedData.frameQueues[modelType].pop();
            }
        }

        // 处理帧数据
        for (const auto& frame : frames) {
            if (modelType == "hbb" && apphbb) {
                // 烟雾检测推理逻辑
                // std::cout << " HBB inference " << std::endl;
                int rtsp_id = frame.rtsp_id;
                RtspRegionParams regionParam;
                //根据camera_id找到对应的配置
                for(auto& region: regionParams){
                    if (rtsp_id == region.camera_id){
                        regionParam = region;
                    }
                }
                HBBProcess(apphbb, frame, regionParam, globalParam ,currentAlarmMap);
            } 
            else if (modelType == "obb" && appobb) {
                // air_bottle 推理逻辑
                // std::cout << " OBB inference " << std::endl;
                int rtsp_id = frame.rtsp_id;
                RtspRegionParams regionParam;
                //根据camera_id找到对应的配置
                for(auto& region: regionParams){
                    if (rtsp_id == region.camera_id){
                        regionParam = region;
                    }
                }
                OBBProcess(appobb, frame, regionParam, globalParam ,currentAlarmMap);
            }
        }

        // 合并当前帧的报警信息
        for (const auto& [alarmType, alarmList] : currentAlarmMap) {
            if (accumulatedAlarmMap.find(alarmType) == accumulatedAlarmMap.end()) {
                accumulatedAlarmMap[alarmType] = json::array();
            }
            for (const auto& alarm : alarmList) {
                accumulatedAlarmMap[alarmType].push_back(alarm);
            }
        }

        // 检查是否需要发送结果
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastSendTime);

        if (elapsedTime >= typeIntervalMap[modelType]) {
            
            if (!accumulatedAlarmMap.empty()) {
                
                // 发送累积的报警信息
                SendHttpRequest(accumulatedAlarmMap, curl, httpcfg);
                
                // 清空累积的报警信息
                accumulatedAlarmMap.clear();
                
                // 更新最后发送时间
                lastSendTime = currentTime;
                
            }
            
            // 更新最后发送时间
            lastSendTime = currentTime;
        }

        // std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    curl_easy_cleanup(curl);
}

void CreateConsumerThreads(SharedData& sharedData, 
                           const std::vector<RtspRegionParams>& regionParams,
                           HBBParam& hbb_cfg,
                           OBBParam& obb_cfg,
                           const GlobalParam& global_cfg,
                           const HTTPParam& httpcfg) {
    std::unordered_map<std::string, std::thread> consumerThreads;

    try {
        // 统计不同的 model_type
        for (const auto& model_type : global_cfg.model_types) {
            // 为每种 model_type 创建一个消费者线程
            if (model_type == "hbb") {
                // std::cout << "hbbhbbhbbhbbhbbhbbhbbhbbhbb" << std::endl;
                std::unique_ptr<AppYolov8> apphbb = InitHBBInfer(hbb_cfg).value();
                consumerThreads[model_type] = std::thread(
                    ConsumerThread, 
                    std::ref(sharedData), 
                    model_type,  // 传递 const std::string&
                    std::move(apphbb),           // 移动所有权
                    std::unique_ptr<AppYolov8obb>(nullptr),  // 传递空的 unique_ptr
                    std::ref(regionParams), 
                    std::ref(global_cfg),
                    std::ref(httpcfg)
                );
                // std::cout << "Consumer thread created for model type: " << model_type << std::endl;
            } 
            else if (model_type == "obb") {
                // std::cout << "obbobbobbobbobbobbobb" << std::endl;
                std::unique_ptr<AppYolov8obb> appobb = InitOBBInfer(obb_cfg).value();
                consumerThreads[model_type] = std::thread(
                    ConsumerThread, 
                    std::ref(sharedData), 
                    model_type,  // 传递 const std::string&
                    std::unique_ptr<AppYolov8>(nullptr),  // 传递空的 unique_ptr
                    std::move(appobb),           // 移动所有权
                    std::ref(regionParams), 
                    std::ref(global_cfg),
                    std::ref(httpcfg)
                );
                // std::cout << "Consumer thread created for model type: " << model_type << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception caught while creating consumer threads: " << e.what() << std::endl;
    }

    // 等待消费者线程结束
    for (auto& [modelType, thread] : consumerThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

