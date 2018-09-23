/***************************************************************************************************

    Company       : Huizhou Desay SV Automotive Co., Ltd.

    Division      : Automotive Electronics, Desay Corporation

    Business Unit : Central Technology

    Department    : Advanced Development (Huizhou)

****************************************************************************************************/

#pragma once

#include "SVPType.h"
#include "SVPSingleton.h"


//实现pcm数据的播放
class SVPPcmPlayer
{
	SVP_SINGLETON_CLASS(SVPPcmPlayer)
protected:
    SVPPcmPlayer();
    ~SVPPcmPlayer();
  public:
    //初始化播放设备
    bool PcmInit(const int32_t channels, const uint32_t sampleRate);
    void PcmExit();
    //播放数据
    bool PlayPcmData(uint8_t* pDate, uint32_t nSize, bool bSilence = false);
    //停止播放
    bool PcmDrain();
    //播放文件
    void Play(const std::string& fileName);
};
