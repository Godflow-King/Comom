/*******************************************

 * Copyright (c) 2014��desay sv

 * All rights reserved

 * File name: md5.h

 * Author: xuyafei

 * Create time: 2017.9.9

 * Description:

 * Modffiy time:

 *******************************************/

#ifndef __RN2_SGMW_CN202S_H_KANZI_PROJECTS_WEATHER_SRC_MD5_H__
#define __RN2_SGMW_CN202S_H_KANZI_PROJECTS_WEATHER_SRC_MD5_H__

#pragma once
#include <string>

using namespace std;

typedef unsigned char    UINT1;
typedef unsigned char *    POINTER;
typedef unsigned short    UINT2;
typedef unsigned int    UINT4;


/* MD5 context */
typedef struct {
    UINT4    state[4];        // state (ABCD)
    UINT4    count[2];        // number of bit, modulo 2 ^ 64
    UINT1    buffer[64];        // input buffer
} MD5_CTX;

void MD5Init(MD5_CTX*);
void MD5Update(MD5_CTX *, UINT1 *,    UINT4);
void MD5Final(UINT1 [16], MD5_CTX *);
std::string MD5Result(UINT1 [16]);
std::string MD5Encode(char* src);

#endif // __RN2_SGMW_CN202S_H_KANZI_PROJECTS_WEATHER_SRC_MD5_H__
