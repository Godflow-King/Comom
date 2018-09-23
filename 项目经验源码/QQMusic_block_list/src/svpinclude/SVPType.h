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

#include "SVPPlatform.h"
#include "SVPLimits.h"

#include <stdint.h>
#include <wchar.h>

#ifdef __cplusplus
    #include <vector>
    #include <list>
    #include <map>
    #include <queue>
    #include <deque>
    #include <set>
    #include <algorithm>
    #include <iostream>
#endif

#include <math.h>

#ifndef NULL
    NULL                    0
#endif


#if defined(SVP_WIN_32) || defined(SVP_WIN_CE)
    #define SVP_HANDLE              HANDLE
#else
    #define SVP_HANDLE              int32_t
#endif

#ifndef max
    #define SVP_Max(a,b)            (((a) > (b)) ? (a) : (b))
#else
    #define SVP_Max(a,b)            max(a,b)
#endif

#ifndef min
    #define SVP_Min(a,b)            (((a) < (b)) ? (a) : (b))
#else
    #define SVP_Min(a,b)            min(a,b)
#endif
