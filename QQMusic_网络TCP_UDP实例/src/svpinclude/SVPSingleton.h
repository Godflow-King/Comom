/******************************************************************************
*
* Company       : Huizhou Desay SV Automotive Co., Ltd.
*
* Division      : Automotive Electronics, Desay Corporation
*
* Business Unit : Central Technology
*
* Department    : Advanced Development (Huizhou)
*
******************************************************************************/

#pragma once

#include <mutex>

/**
 * \brief Singleton class
 */

template <class T>
class SVPSingleton {
  public:
    /**
     * \brief Get instance
     * \return Instance of SVPSingleton<T>
     */
    static T* getInstance() {
        std::lock_guard<std::recursive_mutex> lock(_mtx);

        if (_singleton == nullptr)
            _singleton = new T();

        return _singleton;
    }

    /**
     * \brief Destroy instance
     */
    static void destroyInstance() {
        std::lock_guard<std::recursive_mutex> lock(_mtx);

        if (_singleton != nullptr)
            delete _singleton;

        _singleton = nullptr;
    }

  private:
    SVPSingleton() {}
    virtual ~SVPSingleton() {}

  private:
    static std::recursive_mutex _mtx;
    static T*                   _singleton;
};

template <class T>
std::recursive_mutex SVPSingleton<T>::_mtx;

template <class T>
T* SVPSingleton<T>::_singleton = nullptr;

/**
 * \brief Macro to declare SVPSingleton<T>
 */
#define SVP_SINGLETON_CLASS(T) friend class SVPSingleton<T>;
