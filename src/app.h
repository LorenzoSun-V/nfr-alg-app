/*
* @file:   app.h
* @Author: leo
* @Mail:   jzyleomessi@gmail.com
* @Date:   2024-11-25
* @brief:  该文件主要定义了一些常用的结构体和函数，方便后续的开发
*/
#pragma once
#include "TSingleIns.h"
#include "apiheader.h"
#include <stdarg.h>
#include <iostream>
#include <thread>
#include <string>
#include <deque>
#include <vector>
#include <queue>
#include <map>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <atomic>
#include <nlohmann/json.hpp>
using json = nlohmann::json;


// 错误码定义
std::map<ENUM_ERROR_CODE, std::string>  ErrCodeMapinfo = 
{
        // 使用宏来添加映射，app需要与api错误码保持同步
        // 通用错误码定义
        {ENUM_OK,"ENUM_OK"},
        {ERR_INVALID_PARAM,"ERR_INVALID_PARAM"},
        {ERR_NO_FREE_MEMORY,"ERR_NO_FREE_MEMORY"},
        {ERR_INPUT_INSTANCE_INVALID,"ERR_INPUT_INSTANCE_INVALID"}, 
        {ERR_INPUT_IMAGE_EMPTY,"ERR_INPUT_IMAGE_EMPTY"},              
        {ERR_GET_IMAGE_EMPTY,"ERR_GET_IMAGE_EMPTY"},
        {ERR_DETECT_OBJECT_EMPTY,"ERR_DETECT_OBJECT_EMPTY"},
        {ERR_DETECT_PREDICT_FAIL,"ERR_DETECT_PREDICT_FAIL"},
        {ERR_MODEL_DESERIALIZE_FAIL,"ERR_MODEL_DESERIALIZE_FAIL"},     
        {ERR_MODEL_INPUTPATH_NOT_EXIST,"ERR_MODEL_INPUTPATH_NOT_EXIST"},  

        // 图像错误码定义
        {ERR_INPUT_IP_INVALID,"ERR_INPUT_IP_INVALID"},    
        {ERR_MAKE_PIPE_FAILED,"ERR_MAKE_PIPE_FAILED"},    
        {ERR_MAKE_RTSP_SERVER_FAILED,"ERR_MAKE_RTSP_SERVER_FAILED"},    
        {ERR_CANNOT_OPEN_VIDEOSTREAM,"ERR_CANNOT_OPEN_VIDEOSTREAM"},
        {ERR_OPEN_PUSH_VIDEOSTREAM_FAILED,"ERR_OPEN_PUSH_VIDEOSTREAM_FAILED"},   
        {ERR_PUSH_RTMPSTREAM_FAILED,"ERR_PUSH_RTMPSTREAM_FAILED"},   
        {ERR_INIT_SAVE_VIDEO_FAILED,"ERR_INIT_SAVE_VIDEO_FAILED"},   
        {ERR_SAVE_VIDEO_FILE_FAILED,"ERR_SAVE_VIDEO_FILE_FAILED"},  
        {ERR_FFMEDIA_INIT_FAILED,"ERR_FFMEDIA_INIT_FAILED"},
        {ERR_CREATE_ENCODER_FAILED,"ERR_CREATE_ENCODER_FAILED"},
        {ERR_ENCODER_OPEN_FAILED,"ERR_ENCODER_OPEN_FAILED"},
        {ERR_ENCODER_CURRENT_FRAME_FAILED,"ERR_ENCODER_CURRENT_FRAME_FAILED"},
        {ERR_OPEN_PIPE_NAME_FAILED,"ERR_OPEN_PIPE_NAME_FAILED"},    

        // 工业摄像头专用错误码定义
        // {ERR_MVS_ENUM_IP_ADRESS,"ERR_MVS_ENUM_IP_ADRESS"},              
        // {ERR_MVS_CANNOT_ONLINE_IP,"ERR_MVS_CANNOT_ONLINE_IP"},
        // {ERR_MVS_UNSUPPORT_DEVICE_TYPE,"ERR_MVS_UNSUPPORT_DEVICE_TYPE"},
        // {ERR_MVS_CREATE_HANDLE_FAILED,"ERR_MVS_CREATE_HANDLE_FAILED"},       
        // {ERR_MVS_OPEN_DEVICE_FAILED,"ERR_MVS_OPEN_DEVICE_FAILED"},
        // {ERR_MVS_SET_TRIGGER_FAILED,"ERR_MVS_SET_TRIGGER_FAILED"},
        // {ERR_MVS_GET_PAYLOADSIZE_FAILED,"ERR_MVS_GET_PAYLOADSIZE_FAILED"},
        // {ERR_MVS_SET_BINNING_NUMBER,"ERR_MVS_SET_BINNING_NUMBER"},
        // {ERR_MVS_SET_EXPOSURE_FAILED,"ERR_MVS_SET_EXPOSURE_FAILED"},
        // {ERR_MVS_GET_EXPOSURE_FAILED,"ERR_MVS_GET_EXPOSURE_FAILED"},     
        // {ERR_MVS_SET_EXPOSURE_TIME_FAILED,"ERR_MVS_SET_EXPOSURE_TIME_FAILED"},
        // {ERR_MVS_SET_GAINAUTO_FAILED,"ERR_MVS_SET_GAINAUTO_FAILED"},
        // {ERR_MVS_SET_FRAMERATE_FAILED,"ERR_MVS_SET_FRAMERATE_FAILED"},     
        // {ERR_MVS_START_GET_STREAM_FAILED,"ERR_MVS_START_GET_STREAM_FAILED"},     
        // {ERR_MVS_INVALID_BINNING_PARAM,"ERR_MVS_INVALID_BINNING_PARAM"},
        // {ERR_MVS_SET_BINNING_FAILED,"ERR_MVS_SET_BINNING_FAILED"},
        // {ERR_MVS_GET_FRAME_TIMEOUT,"ERR_MVS_GET_FRAME_TIMEOUT"},     
        // {ERR_MVS_CONVERT_MAT_FAILED,"ERR_MVS_CONVERT_MAT_FAILED"},     
        // {ERR_MVS_DEVICE_DISCONNECT,"ERR_MVS_DEVICE_DISCONNECT"},     
        // {ERR_MVS_STOP_GRABBING_FAILED,"ERR_MVS_STOP_GRABBING_FAILED"},
        // {ERR_MVS_CLOSE_DEVICE_FAILED,"ERR_MVS_CLOSE_DEVICE_FAILED"},
        // {ERR_MVS_DESTORY_HANDLE_FAILED,"ERR_MVS_DESTORY_HANDLE_FAILED"},  
        // {ERR_MVS_CACHE_FRAMES_NOT_READY,"ERR_MVS_CACHE_FRAMES_NOT_READY"},    
        // {ERR_MVS_SET_PIXRGB8FORMAT_FAILED,"ERR_MVS_SET_PIXRGB8FORMAT_FAILED"},  
        // {ERR_MVS_GET_PIXRGB8FORMAT_FAILED,"ERR_MVS_GET_PIXRGB8FORMAT_FAILED"},  

        // 模型推理错误码定义
        {ERR_ROCKCHIP_LOAD_MODEL_FAILED,"ERR_ROCKCHIP_LOAD_MODEL_FAILED"},
        {ERR_ROCKCHIP_RKNN_INIT_FAILED,"ERR_ROCKCHIP_RKNN_INIT_FAILED"},
        {ERR_ROCKCHIP_QUERY_VERSION_FAILED,"ERR_ROCKCHIP_QUERY_VERSION_FAILED"},
        {ERR_ROCKCHIP_QUERY_IN_OUT_HEAD_FAILED,"ERR_ROCKCHIP_QUERY_IN_OUT_HEAD_FAILED"},
        {ERR_ROCKCHIP_QUERY_INPUT_ATTR_FAILED,"ERR_ROCKCHIP_QUERY_INPUT_ATTR_FAILED"},
        {ERR_ROCKCHIP_NOT_UINT8_TYPE,"ERR_ROCKCHIP_NOT_UINT8_TYPE"},
        {ERR_ROCKCHIP_RUN_FAILED,"ERR_ROCKCHIP_RUN_FAILED"},
        {ERR_ROCKCHIP_OUTPUT_GET_FAILED,"ERR_ROCKCHIP_OUTPUT_GET_FAILED"},
        {ERR_NVIDIA_LOAD_MODEL_FAILED,"ERR_NVIDIA_LOAD_MODEL_FAILED"},    
        //网络通信
        {ERR_NETWORK_INPUT_PORT_ERROR,"ERR_NETWORK_INPUT_PORT_ERROR"},
        {ERR_NETWORK_CREATE_SOCKET_ERROR,"ERR_NETWORK_CREATE_SOCKET_ERROR"},
        {ERR_NETWORK_BINNING_PORT_FAILED,"ERR_NETWORK_BINNING_PORT_FAILED"},
        {ERR_NETWORK_LISTEN_SOCKET_FAILED,"ERR_NETWORK_LISTEN_SOCKET_FAILED"},
        {ERR_NETWORK_ACCEPT_CLIENT_FAILED,"ERR_NETWORK_ACCEPT_CLIENT_FAILED"},
        {ERR_NETWORK_DISCONNECT_CLIENT,"ERR_NETWORK_DISCONNECT_CLIENT"},
        {ERR_NETWORK_SENDDATA_ERROR,"ERR_NETWORK_SENDDATA_ERROR"},
        {ERR_NETWORK_RECVDATA_ERROR,"ERR_NETWORK_RECVDATA_ERROR"},
        {ERR_NETWORK_INPUT_SERVERIP_ERROR,"ERR_NETWORK_INPUT_SERVERIP_ERROR"},
        {ERR_NETWORK_SETTING_SERVERIP_ERROR,"ERR_NETWORK_SETTING_SERVERIP_ERROR"},
        {ERR_NETWORK_SETTING_NON_BLOCKING_ERROR,"ERR_NETWORK_SETTING_SERVERIP_ERROR"},

        // 串口通信错误码定义
        {ERR_SERICOM_INPUT_DEVICE_NAME_EMPTY,"ERR_SERICOM_INPUT_DEVICE_NAME_EMPTY"},
        {ERR_SERICOM_OPEN_DEVICE_NAME_FAILED,"ERR_SERICOM_OPEN_DEVICE_NAME_FAILED"},
        {ERR_SERICOM_FCNTL_FUNCTION_FAILED,"ERR_SERICOM_FCNTL_FUNCTION_FAILED"},
        {ERR_SERICOM_UNTERMINAL_DEVICE,"ERR_SERICOM_UNTERMINAL_DEVICE"},
        {ERR_SERICOM_SET_CONFIG_ERROR,"ERR_SERICOM_SET_CONFIG_ERROR"},
        {ERR_SERICOM_RECV_DATA_ERROR,"ERR_SERICOM_RECV_DATA_ERROR"},
        {ERR_SERICOM_SEND_DATA_ERROR,"ERR_SERICOM_SEND_DATA_ERROR"},
        {ERR_SERICOM_FD_CHECK_ERROR,"ERR_SERICOM_FD_CHECK_ERROR"},

        // 读写配置文件错误码定义
        {ERR_PROFILE_INPUT_FILENAME_EMPTY,"ERR_PROFILE_INPUT_FILENAME_EMPTY"},
        {ERR_PROFILE_SEARCH_KEYNAME_EMPTY,"ERR_PROFILE_SEARCH_KEYNAME_EMPTY"},
        {ERR_PROFILE_READ_CONFIG_ERROR,"ERR_PROFILE_READ_CONFIG_ERROR"},
        {ERR_PROFILE_NOT_FOUND_SEARCH_KEYNAME,"ERR_PROFILE_NOT_FOUND_SEARCH_KEYNAME"},
        {ERR_PROFILE_CONFIG_ADD_OPTION_ERROR,"ERR_PROFILE_CONFIG_ADD_OPTION_ERROR"},
        {ERR_PROFILE_CONFIG_WRITE_ERROR,"ERR_PROFILE_CONFIG_WRITE_ERROR"},

        // 红外相机错误码定义
        // {ERR_HWJG_CAMERA_SERVER_INIT_FAILED,"ERR_HWJG_CAMERA_SERVER_INIT_FAILED"},
        // {ERR_HWJG_CAMERA_DEVICE_INIT_FAILED,"ERR_HWJG_CAMERA_DEVICE_INIT_FAILED"},
        // {ERR_HWJG_CAMERA_CONTACT_FAILED,"ERR_HWJG_CAMERA_CONTACT_FAILED"},
        // {ERR_HWJG_GET_CAMERA_PARAM_FAILED,"ERR_HWJG_GET_CAMERA_PARAM_FAILED"},
        // {ERR_HWJG_GET_RECT_TEMPERATURE_FAILED,"ERR_HWJG_GET_RECT_TEMPERATURE_FAILED"},
        // {ERR_HWJG_CAMERA_START_FAILED,"ERR_HWJG_CAMERA_START_FAILED"},
        // {ERR_HWJG_WAITING_TIMESTAMP_UPDATE,"ERR_HWJG_WAITING_TIMESTAMP_UPDATE"},
        // {ERR_HWJG_TEMPERATURE_DATA_NOT_FAILED,"ERR_HWJG_TEMPERATURE_DATA_NOT_FAILED"},
        // {ERR_HWJG_IMAGE_DATA_NOT_FAILED,"ERR_HWJG_IMAGE_DATA_NOT_FAILED"},
        // {ERR_HWJG_DEVICE_DISLINKED,"ERR_HWJG_DEVICE_DISLINKED"},
        // {ERR_HWIR_LOGIN_FAILED,"ERR_HWIR_LOGIN_FAILED"},
        // {ERR_HWIR_INVALID_DETECT_POINT,"ERR_HWIR_INVALID_DETECT_POINT"},
        // {ERR_HWIR_CACHE_TEMP_NOT_READY,"ERR_HWIR_CACHE_TEMP_NOT_READY"}
};

static const char* GetErrorCodeName(ENUM_ERROR_CODE code)
{
    return ErrCodeMapinfo[code].c_str(); 
}

/*时间结构体*/
struct NowTime {
    int msec;
    int year, mon, day; // 日期：年、月、日
    int hour, min, sec; // 时间：时、分、秒
};

// rtsp参数结构体
struct RtspServerParam {
    std::string url;
    int id;
    int switch_flag;
    int dtype;
    std::vector<std::string> model_types;
};

// rtsp区域参数结构体
struct SingalRtspRegionParam
{
    int id;
    std::vector<cv::Point> points;
    int switch_flag;
    float iou_thresh;
};

struct RtspRegionParams{
    int camera_id;
    std::vector<SingalRtspRegionParam> rtspRegionParams;
};

// HBB模型参数结构体
struct HBBParam{
    std::string model_path;
    int model_switch;
    int device_id;
};

// OBB模型参数结构体
struct OBBParam{
    std::string model_path;
    int model_switch;
    int device_id;
};

/*文件存储结构体*/
struct StorageParam
{
    std::string storage_path;
    int max_storage_space=200;//GB
    int log_daliy_storage_space=50;//MB
    int image_daliy_storage_space=100;//MB
    int video_daliy_storage_space=100;//MB
    int storage_expiration_days = 60;//默认天数
};

struct rtspframe{
    cv::Mat frame;
    int rtsp_id;
    std::string timestamp;
    std::vector<std::string> model_types;
};

struct SharedData {
    std::unordered_map<std::string, std::queue<rtspframe>> frameQueues; // 按 model_type 分组的队列
    std::unordered_map<std::string, std::mutex> queueMutexes; // 每个队列的互斥锁
    std::unordered_map<std::string, std::condition_variable> queueCVs; // 每个队列的条件变量
    const size_t MAX_QUEUE_SIZE = 30; // 队列最大长度
    std::atomic<bool> isRunning{true};  // 运行状态标志
    std::atomic<bool> ready{false}; // 消费者线程是否可以开始处理数据
    // 新增：共享的报警队列
    std::map<std::string, json> sharedAlarmMap;
    std::mutex alarmMutex;
    std::unordered_map<std::string, std::chrono::system_clock::time_point> lastProcessedTime;
    // std::unordered_map<std::string, int> lastProcessedTime;  // 记录每个任务上次处理的时间
};

// HTTP 参数结构体
struct HTTPParam
{
    std::string http_server_ip;
};

struct GlobalParam{
    int rtsp_num = 1;
    std::vector<std::string> model_types;
    std::unordered_map<std::string, int> typeIntervalMap;
    float window_high = 0.0;
    int hbb_classnum = 0;
    int obb_classnum = 0;
    int frame_rate = 0; //相机帧率
};

