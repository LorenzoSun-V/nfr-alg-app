#pragma once

#include "common.h"
#include "opencv2/opencv.hpp"

extern "C"
{
    /*
     * @brief 初始化获得句柄
     *
	 * @param  pWeightfile          获得句柄
     * @param  pDeepInstance        返回句柄
     * @param  device_id    设置当前句柄绑定在哪个NPU上,[0,1,2]分别npu核心0，1，2
     *                              设置3为均衡模式，均衡运行3台NPU上(经测试yolov8均衡模式模型不起作用，反而设置单个NPU上推理更快)
     * @param  conf_threshold       输入类别置信度
     * @param  nms_threshold        输入iou置信度
     * @param  model_start_class_id 设置模型起始类别ID,默认ID从0开始
     * @return ENUM_ERROR_CODE      返回错误码
     */    
    ENUM_ERROR_CODE InitYolov8InferenceInstance(
        const char* pWeightfile, 
        void** pDeepInstance,
        const int device_id = 0,
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
	ENUM_ERROR_CODE InferenceYolov8GetDetectResult(
		void* pDeepInstance,
		cv::Mat frame,
        std::vector<DetBox> &detBoxs
    );   
                    
      /*
    * @brief 销毁句柄
    *
    * @param   pDeepInstance         需要销毁的句柄 
    *
    * @return  ENUM_ERROR_CODE       返回0表示成功
    */

    ENUM_ERROR_CODE DestoryYolov8InferenceInstance(
        void** pDeepInstance
    );
}
