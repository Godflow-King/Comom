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

#include <stdlib.h>
#include <stdio.h>

#if defined(_WIN32_WCE) || defined(UNDER_CE) || defined(WINCE)
    #define SVP_WIN_CE
#elif defined(WIN32)
    #define SVP_WIN_32
#elif defined(ANDROID)
    #define SVP_ANDROID
#elif defined(__linux__) || defined(__LINUX__)
    #define SVP_LINUX
#endif

#if defined(UNICODE)
    #define SVP_UNICODE
#endif

#if defined(SVP_WIN_32) || defined(SVP_WIN_CE)
    #include "windows.h"
    #define SVP_Sleep(ms)       Sleep(ms)
    #define SVP_GetLastError()  GetLastError()
#else
    #include <errno.h>
    #include <unistd.h>
    #define SVP_Sleep(ms)       usleep((ms)*1000)
    #define SVP_GetLastError()  errno
#endif

#ifndef ASSERT
    #include <assert.h>
    #define SVP_Assert(x)       assert(x)
#else
    #define SVP_Assert(x)       ASSERT(x)
#endif

#ifdef NDEBUG
    #undef SVP_Assert
    #define SVP_Assert(x)       (x)  // Execute (x) when NDEBUG been defined.
#endif

#if defined(SVP_WIN_32) || defined(SVP_WIN_CE)
    #pragma warning(disable : 4267)
    #pragma warning(disable : 4018)
    #pragma warning(disable : 4355)
    #pragma warning(disable : 4996)
    #pragma warning(disable : 4804)
    #pragma warning(disable : 4244)
    #pragma warning(disable : 4013)
    #pragma warning(disable : 4309)
    #pragma warning(disable : 4103)
#endif

#define SVP_UNUSED(x) (void)x
