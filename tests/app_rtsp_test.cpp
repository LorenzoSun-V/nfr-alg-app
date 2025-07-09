/**
* @file    test.cpp
*
* @brief   硬解码接口测试代码
*
*
* All Rights Reserved.
*/
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include "common.h"
#include "videodec.h"


using namespace cv;
using namespace std;

void ReconnectDevice( const char* pInputRtspUrl, int detype , void *pRtspClientInstance)
{
	sleep(3);
	DestoryRtspClientInstance(&pRtspClientInstance);
	InitRtspClientInstance(pInputRtspUrl, &pRtspClientInstance, detype);
}

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
	    cout<<"param1: the make binary;"<<endl;
	    cout<<"example: ./binary rtspurl"<<endl;
	    exit(-1);
    }
    const char* pInputRtspUrl = argv[1];
	//初始化视频解码
    void * pRtspClientInstance = NULL;
	int detype = 0;
	ENUM_ERROR_CODE eOK = InitRtspClientInstance(pInputRtspUrl, &pRtspClientInstance, detype);
	if(eOK != ENUM_OK || NULL == pRtspClientInstance)
	{
        cout<<"can not get pDecoderInstance!"<<endl;
        return -1;
    } 

    void * pRtspClientInstance2 = NULL;
	eOK = InitRtspClientInstance(pInputRtspUrl, &pRtspClientInstance2, detype);
	if(eOK != ENUM_OK || NULL == pRtspClientInstance2)
	{
        cout<<"can not get pDecoderInstance!"<<endl;
        return -1;
    } 

	std::cout<<"init finshed!"<<std::endl;
	int frame_num=0;
	cv::Mat frame(2000, 2000, CV_8UC3);
	
	int frame_num2=0;
	cv::Mat frame2(2000, 2000, CV_8UC3);

	while (true)
	{	
        //获得解码MAT帧
        eOK = GetOneFrameMatDataFromInstance(pRtspClientInstance, frame);
        if(eOK != ENUM_OK){
        	cout<<"Can not read video data!"<<endl;
			ReconnectDevice(pInputRtspUrl, detype, pRtspClientInstance);
        	//break;
        }
		if(!frame.empty()) {
			cout<<"frame_num1= "<<frame_num<<endl;
			frame_num++;
            std::string filename = "1frame_" + std::to_string(frame_num) + ".jpg";
            cv::imwrite(filename, frame);
		}

		eOK = GetOneFrameMatDataFromInstance(pRtspClientInstance2, frame2);
        if(eOK != ENUM_OK){
        	cout<<"Can not read video data!"<<endl;
			ReconnectDevice(pInputRtspUrl, detype, pRtspClientInstance2);
        	//break;
        }
		if(!frame2.empty()) {
			cout<<"frame_num2= "<<frame_num2<<endl;
			frame_num2++;
            std::string filename = "2frame_" + std::to_string(frame_num) + ".jpg";
            cv::imwrite(filename, frame);
		}

    //    waitKey(20);
	}
	DestoryRtspClientInstance(&pRtspClientInstance);

    std::cout << "Finish !"<<endl;
    return 0;
}


