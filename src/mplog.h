/*
* @file:   mplog.h
* @Author: leo
* @Mail:   jzyleomessi@gmail.com
* @Date:   2024-11-25
* @brief:  该文件定义了日志相关的宏和函数
*/
#pragma once

#include "app.h"
#include "applog.h"

// 定义宏
typedef void                    mpVOID;
typedef int                     mpBOOL;
typedef char                    mpCHAR;
typedef unsigned char           mpBYTE;
typedef int                     mpINT;
typedef unsigned int            mpUINT;
typedef long                    mpLONG;
typedef unsigned long           mpULONG;
typedef unsigned short          mpWORD;
typedef unsigned int            mpDWORD;
typedef unsigned long long      mpQWORD;
typedef float                   mpFLOAT;
typedef double                  mpDOUBLE;

#define mpTRUE                  1
#define mpFALSE                 0
#define mpNULL                  0
#define MP_DWINFINIT            ((mpDWORD)-1)

typedef mpUINT                  mpMsgId;
typedef mpUINT                  mpGroupId;

#define LEV_ERR                "ERR"
#define LEV_WAR                "WAR"
#define LEV_INF                "INF"
#define LEV_DBG                "DBG"

/**
 * @ingroup SUB_GROUP_LOG
 * @brief   
 * @param   pszLevel  log level
 * @param   pszFile in which file
 * @param   iLine   in which line
 * @parma   format  output format
 */
mpVOID MP_Log(const mpCHAR * pszLevel, const mpCHAR * pszFile, mpINT iLine, const mpCHAR * format, ...);
mpVOID SW_INFO(const mpCHAR * pszLevel, const mpCHAR * pszFile, mpINT iLine, const mpCHAR * format, ...);

#define LOG_ERR(fmt, ...)  MP_Log(LEV_ERR, __FILE__, __LINE__, ":%s() " fmt, __FUNCTION__, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) MP_Log(LEV_WAR, __FILE__, __LINE__, ":%s() " fmt, __FUNCTION__, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) MP_Log(LEV_INF, __FILE__, __LINE__, ":%s() " fmt, __FUNCTION__, ##__VA_ARGS__)
#define PF_INFO(fmt, ...) SW_INFO(LEV_INF, __FILE__, __LINE__, ":%s() " fmt, __FUNCTION__, ##__VA_ARGS__)

// hh:mm:ss.xxx [LEV]%s %d format
mpVOID MP_Log(const mpCHAR * pszLevel, const mpCHAR * pszFile, mpINT iLine, const mpCHAR * format, ...)
{
    int len = 0;
    const int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];

    va_list args;// = mpNULL;
    va_start(args, format);

    vsnprintf(buffer, BUFFER_SIZE, format, args);
    va_end(args);
    char loginfo[BUFFER_SIZE];
    sprintf(loginfo, "[%s]%s<%d> %s", pszLevel, pszFile, iLine, buffer);
    AppLog::GetInstance()->WriteLogFileContext( std::string(loginfo) );
}


// 只显示不计入日志
mpVOID SW_INFO(const mpCHAR * pszLevel, const mpCHAR * pszFile, mpINT iLine, const mpCHAR * format, ...)
{
    int len = 0;
    const int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];

    va_list args;// = mpNULL;
    va_start(args, format);

    vsnprintf(buffer, BUFFER_SIZE, format, args);
    va_end(args);
    char loginfo[BUFFER_SIZE];
    sprintf(loginfo, "[%s]%s<%d> %s", pszLevel, pszFile, iLine, buffer);
    std::cout << loginfo << std::endl;
}

