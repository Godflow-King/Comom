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

#ifndef MAX_PATH
    #define SVP_MAX_PATH                2048 // Ref to URL Length
#else
    #define SVP_MAX_PATH                MAX_PATH
#endif

#ifndef INVALID_HANDLE_VALUE
    #define SVP_INVALID_HANDLE_VALUE    (SVP_HANDLE)(long*)-1
#else
    #define SVP_INVALID_HANDLE_VALUE    INVALID_HANDLE_VALUE
#endif

#define SVP_MAX_INT8                    0x7F
#define SVP_MIN_INT8                    (-128)
#define SVP_MAX_INT16                   0x7FFF
#define SVP_MIN_INT16                   (-32768)
#define SVP_MAX_INT32                   0x7FFFFFFF
#define SVP_MIN_INT32                   (-SVP_MAX_INT32 - 1)
#define SVP_MAX_INT64                   0x7FFFFFFFFFFFFFFFLL
#define SVP_MIN_INT64                   (-SVP_MAX_INT64 - 1LL)

#define SVP_MAX_UINT8                   0xFF
#define SVP_MIN_UINT8                   0x00
#define SVP_MAX_UINT16                  0xFFFF
#define SVP_MIN_UINT16                  0x0000
#define SVP_MAX_UINT32                  0xFFFFFFFF
#define SVP_MIN_UINT32                  0x00000000
#define SVP_MAX_UINT64                  0xFFFFFFFFFFFFFFFFLL
#define SVP_MIN_UINT64                  0x0000000000000000LL

#define SVP_MAX_DOUBLE                  1.7976931348623158e+308
#define SVP_MIN_DOUBLE                  2.2250738585072014e-308
#define SVP_ZERO_DELTA_DOUBLE           0.000001

#define SVP_MAX_FLOAT                   3.402823466e+38F
#define SVP_MIN_FLOAT                   1.175494351e-38F
#define SVP_ZERO_DELTA_FLOAT            0.000001f

#define SVP_INVALID_UINT8_VALUE         0xFF
#define SVP_INVALID_UINT16_VALUE        0xFFFF
#define SVP_INVALID_UINT32_VALUE        0xFFFFFFFF
#define SVP_INVALID_UINT64_VALUE        0xFFFFFFFFFFFFFFFFLL
