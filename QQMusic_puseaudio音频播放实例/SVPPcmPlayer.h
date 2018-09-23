#ifndef __SVP_QQMUSIC_PCMPLAYER_H__
#define __SVP_QQMUSIC_PCMPLAYER_H__

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
    bool PlayPcmData(const uint8_t* pDate, uint32_t nSize, bool bSilence = false);
    //停止播放
    bool PcmDrain();
    //Will throw away all data currently in buffers
    bool PcmFlush();
};

#endif
