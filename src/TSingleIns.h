/*
* @file:   TSingleIns.h
* @Author: leo
* @Mail:   jzyleomessi@gmail.com
* @Date:   2024-11-25
* @brief:  该文件是对单例模式的封装，用于管理全局唯一的对象
*/
#pragma once

#include <mutex>

template <typename T>
class TSingleIns {
public:
    // 获取单例对象的静态方法
    static T* GetInstance() {
        std::call_once(onceFlag, []() {
            m_pInstance = new T();
        });
        return m_pInstance;
    }

    // 释放单例对象的静态方法
    static void ReleaseInstance() {
        std::lock_guard<std::mutex> lock(mtx);
        if (m_pInstance != nullptr) {
            delete m_pInstance;
            m_pInstance = nullptr;
        }
    }

protected:
    virtual ~TSingleIns() {
        TSingleIns<T>::ReleaseInstance();
    }

    TSingleIns() = default;

private:
    TSingleIns(const TSingleIns&) = delete; // 禁用复制构造函数
    TSingleIns& operator=(const TSingleIns&) = delete; // 禁用赋值运算符

    static std::once_flag onceFlag; // 用于保证线程安全的初始化
    static T* m_pInstance; // 单例对象的静态指针
    static std::mutex mtx;
};

template <typename T>
std::once_flag TSingleIns<T>::onceFlag;

template <typename T>
std::mutex TSingleIns<T>::mtx;

template <typename T>
T* TSingleIns<T>::m_pInstance = nullptr;
