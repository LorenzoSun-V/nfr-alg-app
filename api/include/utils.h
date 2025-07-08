/*
 * @Description: 集成视觉相关小功能模块
 * @LastEditTime: 2025-07-07 13:21:09
 */
#pragma once

#include "common.h"
#include "opencv2/opencv.hpp"

extern "C"
{
    /*
     * @brief 图片处理接口
     *
	 * @param  pImagePath           输入图片地址 
	 * @param  frame                返回图片frame
     *
     * @return  bool                返回true成功
     */    
    bool ReadFrameFromPath(
        const char* pImagePath, 
        cv::Mat& frame
    );
 
    /*
     * @brief 标记识别框，绘制实例分割结果
     *	   
     * @param   frame                   输入检测图片/返回结果帧 
     * @param   detBoxs                 输入实例分割 检测框和mask结果       
     * 
     * @return  ENUM_ERROR_CODE         返回错误码
     */
    ENUM_ERROR_CODE DrawInstanceSegmentResultForImage(
        cv::Mat &frame, 
        std::vector<SegBox> segBoxs,
        std::vector<cv::Mat> masks
    );

    /*
     * @brief 标记识别框，返回带框图像
     *	   
     * @param   frame                   输入检测图片/返回结果帧 
     * @param   detBoxs                 输入检测框结果       
     * 
     * @return  ENUM_ERROR_CODE         返回错误码
     */
	ENUM_ERROR_CODE DrawRectDetectResultForImage(
	    cv::Mat &frame,
        std::vector<DetBox> detBoxs
    );   

    /*
     * @brief 标记识别框，返回带框图像
     *	   
     * @param   frame                   输入检测图片/返回结果帧 
     * @param   detBoxs                 输入检测框结果       
     * 
     * @return  ENUM_ERROR_CODE         返回错误码
     */
    ENUM_ERROR_CODE DrawPersonKeyPoseResultForImage(
        cv::Mat &frame, 
        std::vector<DetBox> detBoxs
    );

    /*
     * @brief 标记识别框，绘制旋转矩形框
     *	   
     * @param   frame                   输入检测图片/返回结果帧 
     * @param   detBoxs                 输入检测框结果       
     * 
     * @return  ENUM_ERROR_CODE         返回错误码
     */
    void DrawRotatedRectForImage(
        cv::Mat &frame, 
        const std::vector<DetBox> detBoxs
    );


    /*
     * @brief 输出模型检测结果
     *	     
     * @param  pSortInstance         传入模型句柄
     * @param  frame                 输入显示图片    
	 * @param  trackBox              输入获得跟踪框  
	 * @param  trackCount            跟踪框个数   
     * @return  FACE_ERROR_E         返回错误码
     */                    

   int DrawTrackResultToImage(
        cv::Mat frame, 
        TrackBox* pTrackBox,
        int trackCount
    );

    /*
     * @brief 转换旋转目标框结果，矩形坐标到角度坐标
     * @param   detBoxs                 输入检测框结果       
     * 
     * @return std::vector<cv::Point>   返回转换后的角点坐标
     */    
    std::vector<cv::Point> xywhr2xyxyxyxy(
        const DetBox detBox
    );

     /*
    * @brief 输出当前时间戳
    * @return  double         毫秒级
    */
    float GetCurrentTimeStampMS();    

    /*
     * @brief 检查IP地址是否有效
     *
	 * @param   ipaddr              输入字符串IP地址
     *
     * @return  bool                返回true成功
     */   
    int CheckValidIPAddress(
        const std::string& ipaddr
    ); 
    
    /*
     * @brief 提取IP地址存储到unsigned char数组中
     *
	 * @param   ipaddr              输入字符串IP地址
     * @param   iparray             输出字符数字
     *
     * @return  bool                返回true成功
     */ 
    int ExtractIPAddress(
        const std::string& ipaddr, 
        unsigned char ip[4]
    );

    /*
     * @brief 输入十六进制字符数组，
     * @brief 输出转换后的十六进制数据
     * @brief 输出十六进制的长度是输入nsize的一半
     * 
	 * @param   ipaddr              输入字符串IP地址
     * @param   iparray             输出字符数字
     *
     * @return  bool                返回true成功
     */ 
    void ConvertStringToHexArray(
        char *pDest, 
        const char *pSrc, 
        int nSize
    ); 

    /*
     * @brief 输入模型检测的图像和检测区域的坐标点，检测区域的长宽，
     * @brief 将检测区域的坐标转为与输入原始图像相同尺寸的坐标
     * 
	 * @param   image_width         输入检测图像的宽
     * @param   image_height        输入检测图像的高
     * @param   area_points         输入检测区域绘制的坐标点
     * @param   area_width          输入检测区域的图像宽
     * @param   area_height         输入检测区域的图像高
     * @param   converted_points    映射到图像相同尺寸的绘制检测区域的坐标
     * 
     * @return  bool                返回true成功
     */ 
    bool NormalizedPointsToImageSize(
        int image_width, 
        int image_height, 
        std::vector<cv::Point> area_points, 
        int area_width, 
        int area_height, 
        std::vector<cv::Point> &converted_points
    );

    /*
     * @brief 计算目标框与检测区域的百分比，判断是否大于阈值
     * 
	 * @param   detbox              输入识别物体目标的矩形坐标
     * @param   converted_points    输入映射后检测区域的坐标,需要输入多边形的每个点坐标
     * @param   threshold           输入过滤交集的百分比，输入（0~100）
     * 
     * @return  bool                返回true成功
     */ 
    bool CalculateAreaRatio(
        const cv::Rect detbox, 
        const std::vector<cv::Point> converted_points, 
        int threshold
    );

    /*
     * @brief  输入图像文件夹获得图像路径列表
     * 
	 * @param   InputFolder         输入图像文件夹路径
     * 
     * @return  vector              返回图像列表
     */ 
    std::vector<std::string> getImagePaths(
        const std::string& InputFolder
    );

    /*
     * @brief  替换图像后缀为txt后缀
     * 
	 * @param   path         输入图像路径
     * 
     * @return  string       返回替换后缀的路径
     */ 
    std::string replaceImageExtensionWithTxt(const std::string& path);

    /*
     * @brief  在图像名称后加上后缀
     * 
	 * @param   path         输入图像路径
     * @param   suffix_name  输入添加的后缀名称
     * 
     * @return  string       返回加上后缀后的路径
     */ 
    std::string replaceImageOutPath(const std::string& path, std::string suffix_name);

    struct EnhanceParams {
        double clahe_clip_limit = 12.0;         ///< CLAHE 对比度限制参数，值越大增强效果越明显。Clip limit for CLAHE.
        cv::Size clahe_tile_grid_size = {8, 8}; ///< CLAHE 网格大小，决定局部增强的范围。Tile grid size for CLAHE.
        float gamma = 1.08f;                    ///< Gamma 校正系数，<1 增亮，>1 变暗。Gamma correction factor.
        double alpha = 1.08;                    ///< 最终线性增强的对比度系数。Contrast multiplier in convertTo().
        double beta = 20.0;                     ///< 最终线性增强的亮度偏移量。Brightness offset in convertTo().
        bool apply_gaussian_blur = true;        ///< 是否执行高斯模糊以降低高频噪点。Enable Gaussian blur.
        bool apply_nl_means_denoising = true;   ///< 是否执行非局部均值去噪处理。Enable Non-Local Means denoising.
    };
    /**
     * @brief 增强输入图像的亮度、对比度和清晰度。
     *        提供 CLAHE 增强、Gamma 校正、高斯模糊和非局部均值去噪等处理方式，
     *        并允许通过参数灵活配置。
     *
     * Enhance the input image by adjusting its brightness, contrast, and clarity.
     * It includes optional CLAHE enhancement, gamma correction, Gaussian blur, 
     * and Non-Local Means denoising, all configurable via the `EnhanceParams`.
     *
     * @param img 原始图像，支持灰度图或BGR彩色图。
     *            Input image (grayscale or BGR color image).
     *
     * @param params 图像增强参数（见 EnhanceParams 结构体）。
     *               Image enhancement parameters (see EnhanceParams struct).
     *
     * @return 增强后的BGR彩色图像。
     *         The enhanced BGR color image.
     */
    cv::Mat enhance_image(const cv::Mat& img, const EnhanceParams& params);
}
