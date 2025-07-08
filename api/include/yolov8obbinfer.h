#pragma once

#include "common.h"
#include "opencv2/opencv.hpp"

extern "C"
{
    /*
     * @brief 初始化获得句柄
     *
	 * @param  pWeightfile          获得句柄
     * @param  errCode              返回错误码，0表示成功
     * @param  run_device_core      设置当前句柄绑定在哪个设备上,瑞星薇分别npu核心0，1，2,
     * @param  conf_threshold       输入类别置信度
     * @param  nms_threshold        输入iou置信度
     * @param  model_start_class_id 设置模型起始类别ID,默认ID从0开始
     * @return ENUM_ERROR_CODE      返回错误码
     */       
    ENUM_ERROR_CODE InitYolov8obbInferenceInstance(
        const char* pWeightfile, 
        void** pDeepInstance,
        const int run_device_core = 0,
        const float conf_threshold = 0.25, 
        const float nms_threshold = 0.45,
        const int model_start_class_id = 0
    );

   	/*
     * @brief 输出模型检测结果
     *	     
     * @param  pDeepInstance         输入模型句柄
     * @param  frame                 输入检测图片  
	 * @param  detBoxs               返回检测框
     *
     * @return  ENUM_ERROR_CODE      返回错误码
     */
	ENUM_ERROR_CODE InferenceYolov8obbGetDetectResult(
		void* pDeepInstance,
		cv::Mat frame,
        std::vector<DetBox> &detBoxs
    );   

    /*
     * @brief 批量输出模型检测结果
     * @param  pDeepInstance         传入模型句柄
     * @param  batch_images          批量输入批量检测图片，最大数量根据模型batch决定
     * @param  batch_result          批量输出检测结果
     *
     * @return  ENUM_ERROR_CODE      返回错误码
     */
    ENUM_ERROR_CODE BatchInferenceYolov8obbGetDetectResult(
        void* pDeepInstance,
        std::vector<cv::Mat> batch_images,
        std::vector<std::vector<DetBox>> &batch_result
    );  

   /*
    * @brief 销毁句柄
    *
    * @param   pDeepInstance         需要销毁的句柄 
    *
    * @return  ENUM_ERROR_CODE       返回0表示成功
    */

    ENUM_ERROR_CODE DestoryYolov8obbInferenceInstance(
        void** pDeepInstance
    );
}
