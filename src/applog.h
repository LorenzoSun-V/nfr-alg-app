/*
* @file:   applog.h
* @Author: leo
* @Mail:   jzyleomessi@gmail.com
* @Date:   2024-11-25
* @brief:  该文件定义了AppLog类，用于记录日志信息
*/
#pragma once
// #include "app.h"
// #include "appcfg.h"
#include "TSingleIns.h"
#include <time.h>
#include <sys/time.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <iomanip>
#include <sstream>

class AppLog: public TSingleIns<AppLog>
{
friend class TSingleIns<AppLog>;
public:
    AppLog();
    ~AppLog();
    void WriteLogFileContext(const std::string context);
    std::string GetStoragePath(std::string rootPath, std::string className, int limitSpace);   
     
private:
    std::ofstream m_logfile;
    std::string m_lastDate;
    std::string m_storagePath;
    int m_logLimitSpace;

    std::string GetCurrentDate();//获得当前日期，格式2024-03-06
    void CheckWithCleanFileDir(const std::string logpath, int maxsize);
};

AppLog::AppLog( ):m_storagePath("storage/"),m_logLimitSpace(50)
{

}

AppLog::~AppLog( )
{
    if (m_logfile.is_open()) m_logfile.close();
}

std::string AppLog::GetCurrentDate() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d");
    return ss.str();
}

void AppLog::CheckWithCleanFileDir(const std::string strDirPath, int maxMBsize) 
{
    std::cout << "strDirPath: "<< strDirPath << std::endl; 
    if (!std::filesystem::exists(strDirPath))
    {
        std::filesystem::create_directories(strDirPath);
        std::cout << "Created directory: " << strDirPath << std::endl;
    }

    std::filesystem::path dirPath{strDirPath};
    if (!std::filesystem::exists(dirPath) || !std::filesystem::is_directory(dirPath)) {
        std::cout << "Directory does not exist or is not a directory." << std::endl;
        return ;
    }

    uintmax_t totalByteSize= 0;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(dirPath)) {
        if (std::filesystem::is_regular_file(entry)) {
            totalByteSize += std::filesystem::file_size(entry);
        }
    }
    //将总大小从字节转换为MB
    double totalMBSize = static_cast<double>(totalByteSize) / (1024 * 1024);
    if (totalMBSize > maxMBsize) { // Directory size greater than 50MB
        std::filesystem::remove_all(std::filesystem::path{strDirPath}); 
        std::cout << "Deleted directory due to size > "<< maxMBsize <<"MB: " << strDirPath << std::endl;
    } else {
        std::cout << "Directory size is within limit: " << totalMBSize << " MB." << std::endl;
    }

}

std::string AppLog::GetStoragePath(std::string rootPath, std::string className, int limitSpace)
{
    std::string storagePath; 
    if( !rootPath.empty() )
    {
        return storagePath;
    }
    storagePath = rootPath + "/"+ className +"/" + GetCurrentDate() +"/";
    CheckWithCleanFileDir(storagePath, limitSpace);
    return storagePath;
}

void AppLog::WriteLogFileContext(const std::string context) 
{
     // 确保日志文件已打开
    std::string currentDate = GetCurrentDate();
    if (m_lastDate != currentDate) {
        if (m_logfile.is_open()) {
            m_logfile.close();
        }
        m_lastDate = currentDate;

        std::string logpath = GetStoragePath(m_storagePath, "log", m_logLimitSpace) + currentDate + ".log";
        m_logfile.open(logpath, std::ios::out | std::ios::app);
    }

    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");

    std::string dateTime = ss.str();
    if (m_logfile.is_open()) {
        m_logfile << dateTime << " " << context << std::endl;
    }
}
