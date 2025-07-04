/*
 * @FilePath: common.h
 * @Description:错误码定义
 * @LastEditTime: 2025-03-18 19:36:16
 */

#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <cstring>

//模型结果存放数据结构
struct DetBox {
    float x, y, h, w;//目标框左上角坐标x,y和框的长宽h,w
    float confidence;//预测精度
    int classID;//类别ID
    float radian;//弧度
    float keypoints[17][3];
    DetBox() {
        x = 0.0;
        y = 0.0;
        h = 0.0;
        w = 0.0;
        radian = 0.0;
        confidence = 0.0;
        classID = -1;
        memset(&keypoints[0][0], 0x00, sizeof(keypoints));
    }
    bool operator==(const DetBox& other) const {
        return x == other.x && y == other.y && h == other.h && w == other.w &&
               confidence == other.confidence && classID == other.classID;
    }
    
    bool SetDetBox(float x1, float y1, float x2, float y2) {

        x = (x1 > 0) ? x1 : 0.0f;
        y = (y1 > 0) ? y1 : 0.0f;
        w = (x2 > x) ? (x2 - x) : 0.0f;
        h = (y2 > y) ? (y2 - y) : 0.0f;
        if (w == 0.0f || h == 0.0f) {
            confidence = 0.0f;
            radian = 0.0;
            classID = -1;
            return false;
        }     
        return true;
    }
};  

struct SegBox {
    float x, y, h, w;  // topleft_x、topleft_y、w、h
    float confidence;  //预测精度
    int classID;       //类别ID
    float mask[32];    //mask系数
    SegBox() {
        x = 0.0;
        y = 0.0;
        h = 0.0;
        w = 0.0;
        confidence = 0.0;
        classID = -1;
        for (int i = 0; i < 32; i++) {
            mask[i] = 0.0;
        }
    }

    bool operator==(const SegBox& other) const {
        if (x != other.x || y != other.y || h != other.h || w != other.w ||
            confidence != other.confidence || classID != other.classID)
            return false;
        for (int i = 0; i < 32; ++i) {
            if (mask[i] != other.mask[i]) return false;
        }
        return true;
    }
    
    void initBox(float x1, float y1, float x2, float y2) {
        x = (x1 > 0) ? x1 : 0.0f;
        y = (y1 > 0) ? y1 : 0.0f;
        w = (x2 > x) ? (x2 - x) : 0.0f;
        h = (y2 > y) ? (y2 - y) : 0.0f;
        if (w == 0.0f || h == 0.0f) {
           confidence = 0.0;
           classID = -1;
        }
    }
};

struct TrackBox {
    float x, y, h, w;//目标框左上角坐标x,y和框的长宽h,w
    float confidence;//预测精度
    int classID;//类别ID
    int trackID;//跟踪ID，同一目标ID不变，新增目标数字会增加
    TrackBox() {
        x = 0.0f;
        y = 0.0f;
        h = 0.0f;
        w = 0.0f;
        confidence = 0.0f;
        classID = -1;
        trackID = -1;
    }
};

struct ImageData
{
    unsigned char* data;  // 指向图像数据的指针
    int width;
    int height;
    int channels; // 通道数，通常为3（RGB）或1（灰度）
    ImageData() : data(nullptr), width(640), height(640), channels(3) {}
};

struct ImageInfo {
    int width;           // 图像宽度
    int height;          // 图像高度
    int channels;        // 通道数 (通常为3，表示RGB)+
    int isRotate;        //是否旋转图像；0-不变；1-垂直反转；2-水平反转
    //int ImageNum;        //图像编号
    // 构造函数
    ImageInfo()
        : width(640), height(640), channels(3), isRotate(0) {}
};

typedef enum {
    /// 通用错误码定义:范围0x0000-0x00FF
    ///< 正确 
    ENUM_OK=0x0000,
    ///< 参数错误
    ERR_INVALID_PARAM=0x00001,
    ///< 内存不足
    ERR_NO_FREE_MEMORY=0x0002,
    ///< 输入句柄参数无效
    ERR_INPUT_INSTANCE_INVALID=0x0003,
    ///< 输入图像为空 
    ERR_INPUT_IMAGE_EMPTY=0x0004,
    ///< 获得图像为空
    ERR_GET_IMAGE_EMPTY=0x0005,
    ///< 没有检测到目标
    ERR_DETECT_OBJECT_EMPTY=0x0006,
    ///< 预测失败   
    ERR_DETECT_PREDICT_FAIL=0x0007,	
    ///< 模型反序列化失败   
    ERR_MODEL_DESERIALIZE_FAIL=0x0008,
    ///< 输入模型路径不存在
    ERR_MODEL_INPUTPATH_NOT_EXIST=0x0009,
    ///< 未知错误
    ERR_UNKNOWN_ERROR=0x000A,

    ///推拉流相关，错误码定义:范围0x0100-0x01FF  
    ///< 输入检查IP地址输入范围有误
    ERR_INPUT_IP_INVALID=0x0100,
    ///<  创建管道失败
    ERR_MAKE_PIPE_FAILED=0x0101,
    ///<  创建管道失败
    ERR_MAKE_RTSP_SERVER_FAILED=0x0102,
    ///< 无法打开视频流
    ERR_CANNOT_OPEN_VIDEOSTREAM=0x0103,
    ///< 打开视频推流功能失败
    ERR_OPEN_PUSH_VIDEOSTREAM_FAILED=0x0104,
    ///< 视频推流失败
    ERR_PUSH_RTMPSTREAM_FAILED=0x0105,
    ///< 初始化视频保存功能失败
    ERR_INIT_SAVE_VIDEO_FAILED=0x0106,
    ///< 保存视频文件失败
    ERR_SAVE_VIDEO_FILE_FAILED=0x0107,
    ///< 解码初始化失败
    ERR_FFMEDIA_INIT_FAILED=0x0108,
    ///< 创建编码器失败
    ERR_CREATE_ENCODER_FAILED=0x0109,
    ///< 编码器加载参数打开失败
    ERR_ENCODER_OPEN_FAILED=0x010A,
    ///< 编码当前帧失败
    ERR_ENCODER_CURRENT_FRAME_FAILED=0x010B,
    ///< 打开写管道描述名失败
    ERR_OPEN_PIPE_NAME_FAILED=0x010C,
    ///< 拉流时间戳未更新
    ERR_RTSP_TIMESTAMP_NOT_UPDATE=0x0010D,
    ///< 格式上下文或流索引错误
    ERR_FMT_CTX_OR_STREAM_INDEX_ERROR=0x0010E,
    //< 解码当前视频结束
    ERR_END_OF_INPUT_STREAM=0x0010F,
    ///< 到达设置超时时间，操作中断点
    ERR_OPERATION_INTERRUPTED_DUE_TIMEOUT=0x00110,
    ///< 当前帧读取不到数据
    ERR_AV_READ_FRAME_FAILED=0x00111,
    ///< 发送数据包失败
    ERR_AV_SEND_PACKET_FAILED=0x00112,
    ///< 输入视频文件不存在
    ERR_INPUT_VIDEO_FILE_NOT_EXIST=0x00113,
    ///< 打开视频文件失败
    ERR_OPEN_VIDEO_FILE_FAILED=0x00114,
     ///< 解码当前帧失败
    ERR_AV_RECEIVE_FRAME_FAILED=0x00115,
    ///< 没有新的有效帧   
    ERR_NO_NEW_VALID_FRAME=0x00117,

    /// 模型推理错误码定义:范围0x0300-0x03FF
    ///< 加载模型失败
    ERR_ROCKCHIP_LOAD_MODEL_FAILED=0x0300,  
    ///< rknn初始化失败
    ERR_ROCKCHIP_RKNN_INIT_FAILED=0x0301,  
    ///< 询问版本失败
    ERR_ROCKCHIP_QUERY_VERSION_FAILED=0x0302,  
    ///< 询问模型输入输出头失败
    ERR_ROCKCHIP_QUERY_IN_OUT_HEAD_FAILED=0x0303,  
    ///< 询问模型输入属性
    ERR_ROCKCHIP_QUERY_INPUT_ATTR_FAILED=0x0304,  
    ///< 模型不是uint8格式
    ERR_ROCKCHIP_NOT_UINT8_TYPE=0x0305,  
    ///< 运行返回为空
    ERR_ROCKCHIP_RUN_FAILED=0x0306,  
    ///< 运行结束获得结果返回为空
    ERR_ROCKCHIP_OUTPUT_GET_FAILED=0x0307,  
    ///< 加载模型失败
    ERR_NVIDIA_LOAD_MODEL_FAILED=0x0308,  
    /// 网络通信错误码定义:范围0x0400-0x04FF
    ///< 端口输入错误
    ERR_NETWORK_INPUT_PORT_ERROR=0x0400,  

     ///< 创建套结字错误
    ERR_NETWORK_CREATE_SOCKET_ERROR=0x0401,        
     ///< 端口绑定失败
    ERR_NETWORK_BINNING_PORT_FAILED=0x0402,    
     ///< 端口监听失败
    ERR_NETWORK_LISTEN_SOCKET_FAILED=0x0403,  
     ///< 接收客户端链接失败
    ERR_NETWORK_ACCEPT_CLIENT_FAILED=0x0404,    
     ///< 与客户端链接断开
    ERR_NETWORK_DISCONNECT_CLIENT=0x0405,   
     ///< 数据发送出错
    ERR_NETWORK_SENDDATA_ERROR=0x0406,     
     ///< 接收客户端数据失败
    ERR_NETWORK_RECVDATA_ERROR=0x0407, 
    ///< 输入服务器IP地址出错
    ERR_NETWORK_INPUT_SERVERIP_ERROR=0x0408, 
    ///< 设置服务器IP地址出错
    ERR_NETWORK_SETTING_SERVERIP_ERROR=0x0409, 
    ///< 设置非堵塞方式出错
    ERR_NETWORK_SETTING_NON_BLOCKING_ERROR=0x040A, 

    /// 串口通信错误码定义:范围0x0500-0x05FF
    ///< 输入串口名称为空
    ERR_SERICOM_INPUT_DEVICE_NAME_EMPTY=0x0500,  
    ///< 打开串口失败
    ERR_SERICOM_OPEN_DEVICE_NAME_FAILED=0x0501,   
    ///< 控制串口FCNTL功能失败
    ERR_SERICOM_FCNTL_FUNCTION_FAILED=0x0502,   
    ///< 串口为非终端设备
    ERR_SERICOM_UNTERMINAL_DEVICE=0x0503,   
    ///< 配置串口参数出错
    ERR_SERICOM_SET_CONFIG_ERROR=0x0504,   
    ///< 接收数据出错
    ERR_SERICOM_RECV_DATA_ERROR=0x0505,  
    ///< 发送数据出错
    ERR_SERICOM_SEND_DATA_ERROR=0x0506,  
    ///< 检查串口套接字出错
    ERR_SERICOM_FD_CHECK_ERROR=0x0507,  

    /// 读写配置文件错误码定义:范围0x0600-0x06FF
    ///< 名称为空
    ERR_PROFILE_INPUT_FILENAME_EMPTY=0x0600,  
    ///< 输入查询关键字为空
    ERR_PROFILE_SEARCH_KEYNAME_EMPTY=0x0601,  
    ///< 读配置文件错误
    ERR_PROFILE_READ_CONFIG_ERROR=0x0602,  
    ///< 未查询到输入的关键字
    ERR_PROFILE_NOT_FOUND_SEARCH_KEYNAME=0x0603,  
    ///< 添加关键字
    ERR_PROFILE_CONFIG_ADD_OPTION_ERROR=0x0604,   
    ///< 写入文件失败
    ERR_PROFILE_CONFIG_WRITE_ERROR=0x0605,     
    
}ENUM_ERROR_CODE;
