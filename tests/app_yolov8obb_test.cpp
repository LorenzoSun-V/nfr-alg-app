#include <iostream>
#include <opencv2/opencv.hpp>
#include "app_yolov8obb.h"

using namespace std;
using namespace cv;

// 初始化Yolov8模型
std::unique_ptr<AppYolov8obb> InitYolov8obbInfer(string model_path) 
{
    
    std::unique_ptr<AppYolov8obb> appInfer = std::make_unique<AppYolov8obb>();
    bool bret = appInfer->CreateInstance(model_path, 0, 0.25, 0.45, 0);
    if(!bret) {
        LOG_ERR("AppYolov8obb CreateInstance ERROR");
        std::cout<<"AppYolov8obb CreateInstance ERROR"<<std::endl;
        return NULL;
    }
    std::cout<<"InitYolov8obbInfer sucessful!!"<<std::endl;
    return appInfer;
}

int main(int argc, char* argv[]) {

    if(argc < 3){
		cout<<"param1: the make binary;"<<endl;
		cout<<"param2: model path is the model and prototxt path;"<<endl;
		cout<<"param3: input the detect image file;"<<endl;
		cout<<"example: ./binary weightsfile imagepath"<<endl;
		exit(-1);
 	}

    const char* pWeightsfile = argv[1];
    const char* pVideofile = argv[2];

    // 初始化Yolov8模型
    std::unique_ptr<AppYolov8obb> appInfer = InitYolov8obbInfer(pWeightsfile);

    // 读取图片
    Mat img = imread(pVideofile);
    if(img.empty()){
        cout<<"read image file failed!"<<endl;
        return -1;
    }

    // 进行推理
    std::vector<DetBox> result;
   
    bool ret = appInfer->RunModelInference(img,result);
    if(!ret){
        cout<<"run model inference failed!"<<endl;
        return -1;
    }

    // 显示结果
    bool bDraw = appInfer->DrawInferRectResult(img,result);
    if(bDraw){
        imwrite("result.jpg",img);
        waitKey(0);
    }

    return 0;
}

// ./yolov8obb_test /home/kylin/nofar-ai-box/models/obb_class11n_device_0.2_u8.rknn /home/kylin/nofar-ai-box/asserts/img/1750733108561_182.jpg