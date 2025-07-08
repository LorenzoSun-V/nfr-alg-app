#pragma once

#include "app.h"
#include "mplog.h"
#include "yolov8obbinfer.h"
#include "utils.h"
#include <iostream>
#include <string>
#include <dirent.h>
#include <sys/stat.h>

using namespace std;
using namespace cv;

class AppYolov8obb : public TSingleIns<AppYolov8obb> {
friend class TSingleIns<AppYolov8obb>;
public:
    AppYolov8obb();
    virtual ~AppYolov8obb();

    bool RunModelInference(cv::Mat &frame, std::vector<DetBox> &detResult);
    std::string GetModelName() const {
        size_t pos = m_pModelPath.find_last_of("/\\");
        if (pos == std::string::npos) return m_pModelPath;
        return m_pModelPath.substr(pos + 1);
    }
    bool DrawInferRectResult(cv::Mat& frame,  std::vector<DetBox> &detResult);
    bool CreateInstance(std::string strModelPath, const int device_id);
    bool DestoryInstance();

private:
    void* m_pYolov8obbInstance; 
    std::string m_pModelPath;
};

// 构造函数
AppYolov8obb::AppYolov8obb() : m_pYolov8obbInstance(nullptr) {}

// 析构函数
AppYolov8obb::~AppYolov8obb() {
    DestoryInstance();
}

// 创建模型实例
bool AppYolov8obb::CreateInstance(std::string strModelPath, const int device_id) 
{
    //初始化获得句柄
    ENUM_ERROR_CODE code = InitYolov8obbInferenceInstance(strModelPath.c_str(), &m_pYolov8obbInstance, device_id);
    if(ENUM_OK != code) {
        LOG_ERR("InitYolov8InferenceInstance, return %s", GetErrorCodeName(code));
        return false;
    }
    m_pModelPath = strModelPath;
    return true;
}

// 销毁模型实例
bool AppYolov8obb::DestoryInstance() 
{
    ENUM_ERROR_CODE code = DestoryYolov8obbInferenceInstance(&m_pYolov8obbInstance);
    if(ENUM_OK != code) {
        LOG_WARN("DestoryDeepmodeInstance, return %s", GetErrorCodeName(code));
        return false;
    }
    return true;
 }

bool AppYolov8obb::RunModelInference(cv::Mat &frame,  std::vector<DetBox> &detResult) 
{
    if(frame.empty() ) {
        std::cout<<"frame is empty" <<std::endl;
        return false;
    }
    if (!m_pYolov8obbInstance) {
        std::cerr << "m_pYolov8 obb Instance is nullptr!" << std::endl;
        return false;
    }
    ENUM_ERROR_CODE code = InferenceYolov8obbGetDetectResult(m_pYolov8obbInstance, frame, detResult);
    // std::cout << "InferenceYolov8obbGetDetectResult, return " << GetErrorCodeName(code) << std::endl;
    if(ENUM_OK != code) {
        //LOG_ERR("InferenceGetDetectResult, return %s", GetErrorCodeName(code));
        return false;
    }

    return true;    
}


bool AppYolov8obb::DrawInferRectResult(cv::Mat& frame,  std::vector<DetBox> &detResult)
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
        vector<cv::Point> box_points = xywhr2xyxyxyxy(obj);
        // 绘制旋转框的四条边
        for (int i = 0; i < 4; i++) {
            cv::line(frame, box_points[i], box_points[(i + 1) % 4], cv::Scalar(0, 255, 0), 2);
        }
    
    }
    
    return true;
}


