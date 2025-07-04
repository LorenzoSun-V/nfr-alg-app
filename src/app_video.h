#pragma once
#include "app.h"
#include "mplog.h"

class AppVideo
{
    // friend class TSingleIns<AppVideo>;
public:
    AppVideo();
    virtual ~AppVideo();
    //创建RTSP推流接口
    bool CreateRtspDecoder(std::string rtspurl, int detype);
    bool DecodeGetOneFrameLoop(cv::Mat &frame);
    bool DecodeGetOneFrame(cv::Mat &frame);
    bool DestoryRtspDecoder();

private:
    void* m_pRtspDecoderInstance; 
};

AppVideo::AppVideo()
{

};

AppVideo::~AppVideo()
{
    DestoryRtspDecoder();
}

bool AppVideo::CreateRtspDecoder(std::string rtspurl, int detype)
{
    if(rtspurl.empty() ) {
        std::cout<< "Input rtspurl is empty"<<std::endl;
        return false;
    }
    ENUM_ERROR_CODE code = InitRtspClientInstance(rtspurl.c_str(), detype, &m_pRtspDecoderInstance);
    if(code != ENUM_OK ){
        LOG_ERR("InitRtspClientInstance failed, return %s", GetErrorCodeName(code));
        return false;
    }  
    return true;
}

bool AppVideo::DecodeGetOneFrame(cv::Mat &frame)
{
    ENUM_ERROR_CODE code = GetOneFrameMatDataFromInstance(m_pRtspDecoderInstance, frame);
    if(code != ENUM_OK ){
        LOG_ERR("DecodeGetOneFrame failed, return %s", GetErrorCodeName(code));
        return false;
    }  
    return true;
}

bool AppVideo::DecodeGetOneFrameLoop(cv::Mat &frame)
{
    ENUM_ERROR_CODE code = GetOneFrameMatDataLoopFromInstance(m_pRtspDecoderInstance, frame);
    if(code != ENUM_OK ){
        LOG_ERR("DecodeGetOneFrame failed, return %s", GetErrorCodeName(code));
        return false;
    }  
    return true;
}

bool AppVideo::DestoryRtspDecoder()
{
    if(NULL == m_pRtspDecoderInstance) return true;
    ENUM_ERROR_CODE code = DestoryRtspClientInstance(&m_pRtspDecoderInstance);
    if(code != ENUM_OK ){
        LOG_ERR("DestoryRtspDecoder failed, return %s", GetErrorCodeName(code));
        return false;
    }  
    return true;
}

