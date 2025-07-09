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
void ReconnectCamera(std::unique_ptr<AppVideo> &rtspCamera, const RtspServerParam &rtspcfg){
    std::this_thread::sleep_for(std::chrono::seconds(3));
    bool ret_destory = rtspCamera->DestoryRtspDecoder();            
    bool ret_init = rtspCamera->CreateRtspDecoder(rtspcfg.url,rtspcfg.dtype);
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
            frameData.model_names = rtspParam.model_names;
            errorCounter = 0; // 重置错误计数器

            // 将帧放入所有相关类型的队列
            // for (const auto& type : frameData.model_types) {
            //     std::lock_guard<std::mutex> lock(sharedData.queueMutexes[type]);
            //     sharedData.frameQueues[type].push(frameData); // 将帧放入队列
            //     sharedData.ready = true; // 设置 ready 标志
            //     // std::cout << "Producer: Frame with types [" << type << "] from camera " << frameData.rtsp_id << " added to queue." << std::endl;
            //     sharedData.queueCVs[type].notify_one();
            // }
            for (const auto& name : frameData.model_names) {
                std::lock_guard<std::mutex> lock(sharedData.queueMutexes[name]);
                auto& queue = sharedData.frameQueues[name];

                if (queue.size() >= sharedData.MAX_QUEUE_SIZE) {
                    // 队列已满，丢弃最老的帧，确保队列里都是最新的帧
                    queue.pop();
                }
                queue.push(frameData);
                sharedData.ready = true;
                sharedData.queueCVs[name].notify_one();
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
                    ReconnectCamera(appRtsp, rtspParam); // 重新连接 RTSP 流
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
    if (results.size() > 0 && frameData.rtsp_id == 8) {
        // cv::Mat frame_draw = frameData.frame.clone();
        // appfiresmoke->DrawInferRectResult(frame, results);
        std::string img_name = "rtsp_" + std::to_string(frameData.rtsp_id) + "_" + appfiresmoke->GetModelName() + "_" + frameData.timestamp + ".jpg";
        cv::imwrite(img_name, frame);
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
        // cv::Mat frame_draw = frameData.frame.clone();
        // appYolov8OBB->DrawInferRectResult(frame, results);
        std::string img_name = "rtsp_" + std::to_string(frameData.rtsp_id) + "_" + appYolov8OBB->GetModelName() + "_" + frameData.timestamp + ".jpg";
        cv::imwrite(img_name, frame);
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
                    const std::string& modelName,
                    const std::string& modelType,
                    std::unique_ptr<AppYolov8> apphbb,
                    std::unique_ptr<AppYolov8obb> appobb,
                    const std::vector<RtspRegionParams>& regionParams,
                    const ModelParam& modelParam,
                    const GlobalParam& globalParam,
                    const HTTPParam& httpcfg) {
    
    std::cout << "ConsumerThread started for model: " << modelName << " (type: " << modelType << ")" << std::endl;
    
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
        std::map<std::string, json> currentAlarmMap;
        
        // 从帧队列中获取帧 - 使用模型名称作为键
        {
            std::unique_lock<std::mutex> lock(sharedData.queueMutexes[modelName]);
            sharedData.queueCVs[modelName].wait(lock, [&] { 
                return !sharedData.frameQueues[modelName].empty() || !sharedData.isRunning; 
            });
            
            if (!sharedData.isRunning) break;
            
            // 取出所有帧
            while (!sharedData.frameQueues[modelName].empty()) {
                frames.push_back(sharedData.frameQueues[modelName].front());
                sharedData.frameQueues[modelName].pop();
            }
        }
        
        // std::cout << "Model " << modelName << " processing " << frames.size() << " frames" << std::endl;
        
        // 处理帧数据
        for (const auto& frame : frames) {
            int rtsp_id = frame.rtsp_id;
            RtspRegionParams regionParam;
            
            // 根据camera_id找到对应的配置
            bool found = false;
            for(const auto& region : regionParams){
                if (rtsp_id == region.camera_id){
                    regionParam = region;
                    found = true;
                    break;
                }
            }
            
            if (!found) {
                std::cerr << "Region parameter not found for camera_id: " << rtsp_id << std::endl;
                continue;
            }
            
            // 根据模型类型进行推理
            if (modelType == "hbb" && apphbb) {
                // std::cout << "Performing HBB inference for model: " << modelName << std::endl;
                HBBProcess(apphbb, frame, regionParam, globalParam, currentAlarmMap);
            } 
            else if (modelType == "obb" && appobb) {
                // std::cout << "Performing OBB inference for model: " << modelName << std::endl;
                OBBProcess(appobb, frame, regionParam, globalParam, currentAlarmMap);
            }
            else {
                std::cerr << "No valid inference app for model: " << modelName << " (type: " << modelType << ")" << std::endl;
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
        
        // 使用模型类型来获取时间间隔
        auto intervalIt = typeIntervalMap.find(modelType);
        if (intervalIt != typeIntervalMap.end() && elapsedTime >= intervalIt->second) {
            
            if (!accumulatedAlarmMap.empty()) {
                std::cout << "Sending HTTP request for model: " << modelName << std::endl;
                SendHttpRequest(accumulatedAlarmMap, curl, httpcfg);
                accumulatedAlarmMap.clear();
            }
            
            lastSendTime = currentTime;
        }
        
    }
    
    std::cout << "ConsumerThread ended for model: " << modelName << std::endl;
    curl_easy_cleanup(curl);
}

void CreateConsumerThreads(SharedData& sharedData, 
                           const std::vector<RtspRegionParams>& regionParams,
                           const std::vector<ModelParam>& modelParams,
                           const GlobalParam& global_cfg,
                           const HTTPParam& httpcfg) {
    
    std::vector<std::thread> consumerThreads;
    
    try {
        // 为每个模型创建一个消费者线程
        for (const auto& modelParam : modelParams) {
            std::cout << "Creating consumer thread for model: " << modelParam.model_name 
                      << " (type: " << modelParam.model_type << ")" << std::endl;
            
            if (modelParam.model_type == "hbb") {
                // 创建HBB推理实例
                HBBParam hbb_cfg;
                hbb_cfg.model_name = modelParam.model_name;
                hbb_cfg.model_path = modelParam.model_path;
                hbb_cfg.conf_thresh = modelParam.conf_thresh;
                hbb_cfg.nms_thresh = modelParam.nms_thresh;
                hbb_cfg.device = modelParam.device;
                hbb_cfg.model_start_class_id = modelParam.model_start_class_id;
                
                auto apphbb = InitHBBInfer(hbb_cfg);
                if (apphbb.has_value()) {
                    consumerThreads.emplace_back(
                        ConsumerThread, 
                        std::ref(sharedData), 
                        modelParam.model_name,
                        modelParam.model_type,
                        std::move(apphbb.value()),
                        std::unique_ptr<AppYolov8obb>(nullptr),
                        std::ref(regionParams), 
                        std::ref(modelParam),
                        std::ref(global_cfg),
                        std::ref(httpcfg)
                    );
                    std::cout << "HBB consumer thread created for model: " << modelParam.model_name << std::endl;
                } else {
                    std::cerr << "Failed to initialize HBB model: " << modelParam.model_name << std::endl;
                }
            } 
            else if (modelParam.model_type == "obb") {
                // 创建OBB推理实例
                OBBParam obb_cfg;
                obb_cfg.model_name = modelParam.model_name;
                obb_cfg.model_path = modelParam.model_path;
                obb_cfg.conf_thresh = modelParam.conf_thresh;
                obb_cfg.nms_thresh = modelParam.nms_thresh;
                obb_cfg.device = modelParam.device;
                obb_cfg.model_start_class_id = modelParam.model_start_class_id;
                
                auto appobb = InitOBBInfer(obb_cfg);
                if (appobb.has_value()) {
                    consumerThreads.emplace_back(
                        ConsumerThread, 
                        std::ref(sharedData), 
                        modelParam.model_name,
                        modelParam.model_type,
                        std::unique_ptr<AppYolov8>(nullptr),
                        std::move(appobb.value()),
                        std::ref(regionParams), 
                        std::ref(modelParam),
                        std::ref(global_cfg),
                        std::ref(httpcfg)
                    );
                    std::cout << "OBB consumer thread created for model: " << modelParam.model_name << std::endl;
                } else {
                    std::cerr << "Failed to initialize OBB model: " << modelParam.model_name << std::endl;
                }
            }
            else {
                std::cerr << "Unknown model type: " << modelParam.model_type << " for model: " << modelParam.model_name << std::endl;
            }
        }
        
        std::cout << "Created " << consumerThreads.size() << " consumer threads" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception caught while creating consumer threads: " << e.what() << std::endl;
    }
    
    // 等待所有消费者线程结束
    for (auto& thread : consumerThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

