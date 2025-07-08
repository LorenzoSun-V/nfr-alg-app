#pragma once

#include "app.h"
#include "mplog.h"
#include "yolov8infer.h"
#include <iostream>
#include <string>
#include <dirent.h>
#include <sys/stat.h>

class AppYolov8 : public TSingleIns<AppYolov8>
{ 
friend class TSingleIns<AppYolov8>;

public:
    AppYolov8();
    virtual ~AppYolov8();

    bool RunModelInference(cv::Mat &frame, std::vector<DetBox> &detResult);
    std::string GetModelName() const {
        size_t pos = m_pModelPath.find_last_of("/\\");
        if (pos == std::string::npos) return m_pModelPath;
        return m_pModelPath.substr(pos + 1);
    }
    bool DrawInferRectResult(cv::Mat &frame,  std::vector<DetBox> &detResult);
    bool CreateInstance(std::string strModelPath, const int device_id, 
                        const float conf_threshold, const float nms_threshold, 
                        const int model_start_class_id);
    bool DestoryInstance();

private:
    void* m_pYolov8Instance; 
    std::string m_pModelPath;

};

// 构造函数
AppYolov8::AppYolov8() : m_pYolov8Instance(NULL)
{
}

// 析构函数
AppYolov8::~AppYolov8()
{
    DestoryInstance();
}

// 创建模型实例
bool AppYolov8::CreateInstance(std::string strModelPath, const int device_id, 
                               const float conf_threshold, const float nms_threshold, 
                               const int model_start_class_id) 
{
    //初始化获得句柄
    ENUM_ERROR_CODE code = InitYolov8InferenceInstance(strModelPath.c_str(), &m_pYolov8Instance,
                                                       device_id, conf_threshold, nms_threshold, model_start_class_id);
    if(ENUM_OK != code) {
        LOG_ERR("InitYolov8InferenceInstance, return %s", GetErrorCodeName(code));
        return false;
    }
    m_pModelPath = strModelPath;
    return true;
}

// 销毁模型实例
bool AppYolov8::DestoryInstance() 
{
    ENUM_ERROR_CODE code = DestoryYolov8InferenceInstance(&m_pYolov8Instance);
    if(ENUM_OK != code) {
        LOG_WARN("DestoryDeepmodeInstance, return %s", GetErrorCodeName(code));
        return false;
    }
    return true;
 }

bool AppYolov8::RunModelInference(cv::Mat &frame,  std::vector<DetBox> &detResult) 
{
    if(frame.empty() ) {
        std::cout<<"frame is empty" <<std::endl;
        return false;
    }
    // std::cout << "Frame channels: " << frame.channels() << std::endl;
    // std::cout << "Frame type: " << frame.type() << std::endl;
    if (!m_pYolov8Instance) {
    std::cerr << "m_pYolov8 Instance is nullptr!" << std::endl;
    return false;
    }
    // std::cout << "Yolov8  RunModelInference" << std::endl;
    ENUM_ERROR_CODE code = InferenceYolov8GetDetectResult(m_pYolov8Instance, frame, detResult);
    
    if(ENUM_OK != code) {
        //LOG_ERR("InferenceGetDetectResult, return %s", GetErrorCodeName(code));
        return false;
    }

    return true;    
}


bool AppYolov8::DrawInferRectResult(cv::Mat& frame,  std::vector<DetBox> &detResult)
{
    if (frame.empty()) {
        std::cout <<  "frame is empty." << std::endl;
        return false;
    }
    
    if( detResult.empty() )
    {
    	std::cout << "detResult is empty!" << std::endl;
	    return false;
    }
    
    for (size_t i = 0; i < detResult.size(); i++)
    {
        const DetBox& obj = detResult[i];
        cv::rectangle(frame, cv::Rect(obj.x, obj.y, obj.w, obj.h), cv::Scalar(0, 0, 255), 5, 8);
    
    }
    
    return true;
}


