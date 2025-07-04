/*
* @file:   appcfg.h
* @Author: leo
* @Mail:   jzyleomessi@gmail.com
* @Date:   2024-11-25
* @brief:  该文件是应用配置文件的头文件，主要包含配置文件的读写接口
*/
#pragma once
#include "app.h"
#include "mplog.h"

class AppConfig: public TSingleIns<AppConfig>
{
friend class TSingleIns<AppConfig>;
public:
    AppConfig();
    virtual ~AppConfig();


    bool SetConfigData(const std::string strAppName, const std::string strKeyName, const std::string strKeyVal);
    
    bool CreateInstance(const char* pConfigfilePath);
    
    std::string GetOneLineStringValue(std::string strAppName,std::string strKeyName);
    bool HasKey(const std::string& section, const std::string& key);
    int GetConfigValueAsInt(const std::string strAppName, const std::string strKeyName);
    float GetConfigValueAsFloat(const std::string strAppName, const std::string strKeyName);
    float GetConfigValuePercentToDecimalsAsFloat(const std::string strAppName, const std::string strKeyName);
private:
    bool DestoryInstance();

private:
    void* m_pProfileInstance;  //配置文件句柄
    const char* m_pConfigfilePath; //配置文件路径
};

AppConfig::AppConfig(): m_pProfileInstance(NULL)  {};

AppConfig::~AppConfig()
{
    DestoryInstance();
}


bool AppConfig::CreateInstance(const char*  pConfigfilePath) 
{
    //初始化获得句柄    
    ENUM_ERROR_CODE code = InitYamlInstance(pConfigfilePath, &m_pProfileInstance);
    if(ENUM_OK != code) 
    {
        LOG_ERR("InitYamlInstance failed, return %s", GetErrorCodeName(code));
        return false;
    }
    m_pConfigfilePath = pConfigfilePath;

    return true;
}

bool AppConfig::DestoryInstance() 
{
    ENUM_ERROR_CODE code = DestorytYamlInstance(&m_pProfileInstance);
    if(ENUM_OK != code) 
    {
        LOG_ERR("DestorytYamlInstance failed, return %s", GetErrorCodeName(code));
        return false;
    }
    return true;
 }

std::string AppConfig::GetOneLineStringValue(std::string strAppName,std::string strKeyName)
{
    std::string strValue;
    char context[1024];
    // std::cout << strAppName << " " << strKeyName << std::endl;
    ENUM_ERROR_CODE code = GetYamlString(m_pProfileInstance, strAppName.c_str(), strKeyName.c_str(), context, sizeof(context));
    if (ENUM_OK != code) 
    {
        LOG_ERR("[strAppName(%s)]strKeyName(%s), return %s", strAppName.c_str(), strKeyName.c_str(), GetErrorCodeName(code));
        return strValue;
    } 
    return std::string(context);
}

int AppConfig::GetConfigValueAsInt(const std::string strAppName, const std::string strKeyName)
{
    return atoi(GetOneLineStringValue(strAppName, strKeyName).c_str());
}

//获取映射表中值转为浮点
float AppConfig::GetConfigValueAsFloat(const std::string strAppName, const std::string strKeyName)
{
    return atof( GetOneLineStringValue(strAppName, strKeyName).c_str());
}

float AppConfig::GetConfigValuePercentToDecimalsAsFloat(const std::string strAppName, const std::string strKeyName)
{
    return atof( GetOneLineStringValue(strAppName, strKeyName).c_str() )/100.0;
}

bool AppConfig::SetConfigData(std::string strAppName, std::string strKeyName,std::string strKeyVal)
{
    /*更新配置文件中的值*/
    ENUM_ERROR_CODE code = SetYamlString(m_pProfileInstance, strAppName.c_str(), strKeyName.c_str(), strKeyVal.c_str() );
    if (ENUM_OK != code) 
    {
        LOG_ERR("[strAppName(%s)]strKeyName(%s), return %s", strAppName.c_str(), strKeyName.c_str(), GetErrorCodeName(code));
        return false;
    }
    std::cout << "Setting sucessful "<<std::endl;
    return true;
}

bool AppConfig::HasKey(const std::string& section, const std::string& key){
    std::string value = GetOneLineStringValue(section, key);
    return !value.empty();
}
