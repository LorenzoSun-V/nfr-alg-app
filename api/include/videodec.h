#pragma once

#include "common.h"
#include "opencv2/opencv.hpp"

extern "C"
{
 	/*
     * @brief 输入rtsp流初始化实例
     *
     * @param  rtspurl                输入rtspurl流地址
     * @param  pRtspClientInstance    返回初始化成功后的句柄指针       
     * @param  video_type             0:配置RTSP流解码 1:配置视频文件解码 
     * @param  timeout                配置断网超时响应时间,默认5s后响应             
     * @return ENUM_ERROR_CODE        返回错误码                  
     */	     
    ENUM_ERROR_CODE InitRtspClientInstance(
        const char* rtspurl,
        void** pRtspClientInstance, 
        const int video_type = 0,
        const int timeout = 5
    );	

 	/*
     * @brief 从实例中获取一帧mat数据
     *
     * @param  pRtspClientInstance    输入句柄指针
     * @param  frame                  输入待发送的mat帧                    
     * @return ENUM_ERROR_CODE        返回错误码                  
     */	     
    ENUM_ERROR_CODE GetOneFrameMatDataFromInstance(
        void* pRtspClientInstance,
        cv::Mat& frame
    );	

 	/*
     * @brief 销毁实例
     *
     * @param  pRtspClientInstance    输入句柄指针                          
     * @return ENUM_ERROR_CODE        返回错误码                  
     */	   
    ENUM_ERROR_CODE DestoryRtspClientInstance( 
        void **pRtspClientInstance
    );
}
