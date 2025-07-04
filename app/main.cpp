#include "mplog.h"
#include "app_init.h"
#include "app.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <chrono>
#include <thread>
#include "app_loop.h"
#include <curl/curl.h>

int main(int argc, char* argv[])
{
    const char* jsonfile = argv[1];
    
    try {
        
        // InitializeCurl(); // 初始化 libcurl
        
        std::vector<RtspServerParam> rtspParams;
        std::vector<RtspRegionParams> rtspRegionParams;

        SharedData sharedData;
        sharedData.isRunning = true;

        // 模型参数...
        HBBParam hbb_cfg;
        OBBParam obb_cfg;

        GlobalParam globalcfg;
        // NTPParam ntpcfg;
        HTTPParam httpcfg;

        std::map<std::string, std::map<std::string, json>> globalAlarmMap;

        InitConfigParam(jsonfile, rtspParams, rtspRegionParams, hbb_cfg, obb_cfg, sharedData, globalcfg, httpcfg);
        // InitConfigParam(jsonfile, rtspParams, rtspRegionParams, hbb_cfg, obb_cfg, sharedData, globalcfg, ntpcfg, httpcfg);
        
        std::vector<std::string> all_model_type = globalcfg.model_types;
        for (const auto& type : all_model_type) {
            sharedData.frameQueues[type];
            sharedData.queueMutexes[type];
            sharedData.queueCVs[type];
            sharedData.sharedAlarmMap[type];
        }

        // 初始化rtsp流...
        std::vector<std::unique_ptr<AppVideo>> appRtspList;
        for (const auto& rtspParam : rtspParams) {
            auto appRtsp = InitRTSP(rtspParam);
            if (!appRtsp) {
                throw std::runtime_error("Failed to initialize RTSP stream");
            }
            appRtspList.push_back(std::move(appRtsp));
        }

        
        // 启动生产者线程...
        std::vector<std::thread> producerThreads;
        for (size_t i = 0; i < rtspParams.size(); ++i) {
            producerThreads.emplace_back(RtspProducerThread, std::ref(sharedData), std::ref(rtspParams[i]), std::move(appRtspList[i]), std::ref(globalcfg));
        }
        
        // 启动消费者线程
        CreateConsumerThreads(sharedData, rtspRegionParams, hbb_cfg, obb_cfg, globalcfg, httpcfg);
        // 等待线程结束
        for (auto& thread : producerThreads) {
            if (thread.joinable()) {
                thread.join();
            }
        }

        
    }
    catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
    }
    return 0;
}

// export LD_LIBRARY_PATH=/home/kylin/rk-libs/
// sudo cat /sys/kernel/debug/rknpu/load
