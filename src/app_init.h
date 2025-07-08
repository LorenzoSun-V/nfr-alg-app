#ifndef APP_INIT_H
#define APP_INIT_H

#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include "app.h"
#include "app_yolov8.h"
#include "app_yolov8obb.h"
#include "app_video.h"

using json = nlohmann::json;

// 初始化RTSP参数
inline void InitRtspParams(const json& j_camera, RtspServerParam& rtsp) {
    // 从Camera配置项提取RTSP参数
    rtsp.url = j_camera["url"].get<std::string>();
    rtsp.id = j_camera["id"].get<int>();
    rtsp.switch_flag = j_camera["switch"].get<int>();
    rtsp.dtype = j_camera["dtype"].get<int>();  // 0: rtsp 1: video
    // std::cout << "InitRtspParams successful!!" << std::endl;
    // 提取模型类型：遍历models中的obb/hbb，根据switch标志决定是否启用
    rtsp.model_types.clear();
    rtsp.model_names.clear();
    for (const auto& model : j_camera["models"]) {
        // std::cout << model["name"].get<std::string>() << std::endl;
        rtsp.model_names.push_back(model["name"].get<std::string>());
    }
    
}

// 初始化RTSP区域参数
inline void InitRtspRegionParams(const json& j_region, SingalRtspRegionParam& region) {
    
    region.id = j_region["label"].get<int>();

    region.switch_flag = j_region["switch"].get<int>();
 
    region.iou_thresh = j_region["iou"].get<float>();

    // 提取区域点坐标
    region.points.clear();
    for (const auto& temp_point : j_region["Points"]) {
        cv::Point p;
        p.x = temp_point["x"].get<float>();
        p.y = temp_point["y"].get<float>();
        region.points.push_back(p);
    }
}

// rtsp流初始化
inline std::unique_ptr<AppVideo> InitRTSP(const RtspServerParam& rtsp) {
    std::unique_ptr<AppVideo> apprtsp = std::make_unique<AppVideo>();
    bool bret = apprtsp->CreateRtspDecoder(rtsp.url, rtsp.dtype);
    if (!bret) {
        LOG_ERR("AppVideo CreateInstance ERROR");
        std::cout << "AppVideo CreateInstance ERROR" << std::endl;
        return nullptr;
    }
    std::cout << "InitRTSP successful!!" << std::endl;
    return apprtsp;
}

// 初始化配置参数
inline void InitConfigParam(const char* jsonfile, 
                            std::vector<RtspServerParam>& rtspParams, 
                            std::vector<RtspRegionParams>& rtspRegionParams,
                            std::vector<ModelParam>& modelParams,
                            // HBBParam& hbb_cfg, 
                            // OBBParam& obb_cfg,
                            SharedData& rtsp_sharedData, 
                            GlobalParam& global_param,  
                            HTTPParam& httpcfg) 
{
    // 1. 加载JSON文件
    std::ifstream file(jsonfile);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open JSON file: " << jsonfile << std::endl;
        return;
    }
    std::cout << "Loading JSON file: " << jsonfile << std::endl;

    json j;
    file >> j;

    // 2. 初始化RTSP参数（对应JSON中的Camrea数组）
    rtspParams.clear();
    for (const auto& j_camera : j["Camrea"]) {
        RtspServerParam rtsp;
        InitRtspParams(j_camera, rtsp);
        rtspParams.push_back(rtsp);
    }
    std::cout << "RTSP参数初始化成功! 数量: " << rtspParams.size() << std::endl;

    // 3. 初始化RTSP区域参数（遍历所有模型的Points）
    rtspRegionParams.clear();
    for (const auto& j_camera : j["Camrea"]) {
        RtspRegionParams rtspRegionParam;
        rtspRegionParam.camera_id = j_camera["id"];
        for (const auto& model : j_camera["models"]) {
            for(const auto& region_cfg : model["info"]){
                SingalRtspRegionParam region;
                InitRtspRegionParams(region_cfg, region);
                rtspRegionParam.rtspRegionParams.push_back(region);
            }
        }
        rtspRegionParams.push_back(rtspRegionParam);
    }
    std::cout << "RTSP区域参数初始化成功! 数量: " << rtspRegionParams.size() << std::endl;
    // 4. 初始化模型参数（从全局models字段提取）
    for (const auto& model : j["models"]) {
        // if (model["name"] == "hbb") {
        //     hbb_cfg.model_path = model["path"].get<std::string>();
        //     // global_param.hbb_classnum = model["classnum"].get<int>();
        //     hbb_cfg.model_start_class_id = model["model_start_class_id"].get<int>();
        //     hbb_cfg.conf_thresh = model["conf_thresh"].get<float>();
        //     hbb_cfg.nms_thresh = model["nms_thresh"].get<float>();
        //     hbb_cfg.device = model["device"].get<int>();
        //     global_param.model_types.push_back("hbb");
        //     global_param.typeIntervalMap["hbb"] = model["time"].get<int>();
        //     hbb_cfg.model_switch = model["switch"].get<int>();
        // }
        // if (model["name"] == "obb") {
        //     obb_cfg.model_path = model["path"].get<std::string>();
        //     // global_param.obb_classnum = model["classnum"].get<int>();
        //     obb_cfg.model_start_class_id = model["model_start_class_id"].get<int>();
        //     obb_cfg.conf_thresh = model["conf_thresh"].get<float>();
        //     obb_cfg.nms_thresh = model["nms_thresh"].get<float>();
        //     obb_cfg.device = model["device"].get<int>();
        //     global_param.model_types.push_back("obb");
        //     global_param.typeIntervalMap["obb"] = model["time"].get<int>();
        //     obb_cfg.model_switch = model["switch"].get<int>();
        // }
        if (model["type"] == "hbb") {
            ModelParam modelParam;
            modelParam.model_path = model["path"].get<std::string>();
            modelParam.model_start_class_id = model["model_start_class_id"].get<int>();
            modelParam.conf_thresh = model["conf_thresh"].get<float>();
            modelParam.nms_thresh = model["nms_thresh"].get<float>();
            modelParam.device = model["device"].get<int>();
            modelParam.model_switch = model["switch"].get<int>();
            modelParam.model_type = "hbb";
            modelParam.model_name = model["name"].get<std::string>();
            global_param.model_types.push_back("hbb");
            global_param.model_names.push_back(model["name"].get<std::string>());
            // global_param.typeIntervalMap["hbb"] = model["time"].get<int>();
            global_param.typeIntervalMap[model["name"].get<std::string>()] = model["time"].get<int>();
            modelParams.push_back(modelParam);
        }
        else if (model["type"] == "obb") {
            ModelParam modelParam;
            modelParam.model_path = model["path"].get<std::string>();
            modelParam.model_start_class_id = model["model_start_class_id"].get<int>();
            modelParam.conf_thresh = model["conf_thresh"].get<float>();
            modelParam.nms_thresh = model["nms_thresh"].get<float>();
            modelParam.device = model["device"].get<int>();
            modelParam.model_switch = model["switch"].get<int>();
            modelParam.model_type = "obb";
            modelParam.model_name = model["name"].get<std::string>();
            global_param.model_types.push_back("obb");
            global_param.model_names.push_back(model["name"].get<std::string>());
            // global_param.typeIntervalMap["obb"] = model["time"].get<int>();
            global_param.typeIntervalMap[model["name"].get<std::string>()] = model["time"].get<int>();
            modelParams.push_back(modelParam);
        }
        else {
            std::cerr << "Unknown model type: " << model["type"].get<std::string>() << std::endl;
        }
    }

    // 5. 初始化共享数据和其他参数
    global_param.frame_rate = j["frame_rate"].get<int>();
    global_param.window_high = j["Set_Window"].get<int>(); // 假设Set_Window为数值

    // http参数初始化
    httpcfg.http_server_ip = j["HTTP"].get<std::string>(); // 示例中HTTP字段为完整URL
    
    std::cout << "JSON配置加载成功! 摄像头数量: " << rtspParams.size() << std::endl;
}

// 统一错误处理函数
template <typename T>
inline std::optional<std::unique_ptr<T>> HandleInitialization(const std::string& moduleName, bool success, const std::string& errorMessage) {
    if (!success) {
        std::cerr << moduleName << " initialization failed: " << errorMessage << std::endl;
        return std::nullopt;
    }
    std::cout << moduleName << " initialization successful!" << std::endl;
    return std::make_unique<T>();
}

// 通用模型初始化模板函数
template <typename AppType, typename ParamType>
inline std::optional<std::unique_ptr<AppType>> InitModelInfer(const std::string& modelName, ParamType& modelcfg, const std::string& modelPathKey) {
    if (modelcfg.model_switch == 0) {
        std::cerr << modelName << " switch is off!" << std::endl;
        return std::nullopt;
    }

    auto appInfer = HandleInitialization<AppType>(modelName, true, "");
    if (!appInfer) {
        return std::nullopt;
    }

    bool bret = (*appInfer)->CreateInstance(modelcfg.model_path, 
                                            modelcfg.device, 
                                            modelcfg.conf_thresh, 
                                            modelcfg.nms_thresh, 
                                            modelcfg.model_start_class_id);
    if (!bret) {
        return HandleInitialization<AppType>(modelName, false, "CreateInstance failed");
    }
    return appInfer;
}

// HBB模型初始化
inline std::optional<std::unique_ptr<AppYolov8>> InitHBBInfer(HBBParam& modelcfg) {
    return InitModelInfer<AppYolov8>("hbb", modelcfg, "model_path");
}

// OBB模型初始化
inline std::optional<std::unique_ptr<AppYolov8obb>> InitOBBInfer(OBBParam& modelcfg) {
    return InitModelInfer<AppYolov8obb>("obb", modelcfg, "model_path");
}

#endif // APP_INIT_H