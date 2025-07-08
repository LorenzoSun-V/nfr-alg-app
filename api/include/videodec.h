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
     * @param  decoder_type           0:配置RTSP流解码 1:配置视频文件解码 
     * @param  timeout_reconnect      配置断网超时响应时间，默认10s后重连  
     * @param  target_fps             配置目标解码器帧率
     * @param  only_key_frame         是否只解码关键帧，默认false          
     * @return ENUM_ERROR_CODE        返回错误码                  
     */	     
    ENUM_ERROR_CODE InitRtspClientInstance(
        const char* rtspurl,
        void** pRtspClientInstance, 
        const int decoder_type = 0,
        const int timeout_reconnect = 10,
        const int target_fps = 1,
        const bool only_key_frame = false
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
