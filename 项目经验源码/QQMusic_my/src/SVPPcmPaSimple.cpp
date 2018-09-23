/***************************************************************************************************

    Company       : Huizhou Desay SV Automotive Co., Ltd.

    Division      : Automotive Electronics, Desay Corporation

    Business Unit : Central Technology

    Department    : Advanced Development (Huizhou)

****************************************************************************************************/

#include "SVPPcmPlayer.h"
#include <pulse/simple.h>
#include "config.h"

#ifdef SVP_LOG_TAG
    #undef SVP_LOG_TAG
#endif
#define SVP_LOG_TAG     "QQMusic|pcmplayer"

#define KEY_DELAY_TIME  150000
#define FILE_BUF_LENGTH 204800

static const char*       g_appName = "qqmusic";
static pa_simple*       g_pSimple = NULL;
static pa_sample_spec   g_sampleSpec =
{
    .format = PA_SAMPLE_S16LE,
    .rate = 16000,
    .channels = 1
};

SVPPcmPlayer::SVPPcmPlayer()
{
}

SVPPcmPlayer::~SVPPcmPlayer()
{
	PcmExit();
}

void SVPPcmPlayer::PcmExit()
{
    if (g_pSimple)
    {
        pa_simple_free(g_pSimple);
        g_pSimple = NULL;
    }
}

bool SVPPcmPlayer::PcmInit(const int32_t channels, const uint32_t sampleRate)
{
    if (g_pSimple)
        return true;

    int pa_error;
    g_sampleSpec =
    {
        .format = PA_SAMPLE_S16LE,
        .rate = sampleRate,
        .channels = channels 
    };

    g_pSimple = pa_simple_new(NULL, g_appName, PA_STREAM_PLAYBACK, NULL, "playback", &g_sampleSpec, NULL, NULL, &pa_error);
//	g_pSimple = pa_simple_new(NULL, g_appName, PA_STREAM_PLAYBACK, "Navigation", SVPAudioString::SV_AS_LINK_MUSIC.data(), &g_sampleSpec, NULL, NULL, NULL);

    if (NULL == g_pSimple) {
        //SVP_ERROR("pa_simple_new() - rate[%d], channels[%d] fail: %s!", g_sampleSpec.rate, g_sampleSpec.channels, strerror(errno));
        SVP_ERROR("pa_simple_new fail:%s !!", strerror(errno));
        return false;
    }

    SVP_INFO("PcmInit(%d, %u)", channels, sampleRate);
    return true;
}

bool SVPPcmPlayer::PlayPcmData(uint8_t* pDate, uint32_t nSize, bool bSilence) {
    if (NULL == pDate || 0 == nSize || NULL == g_pSimple) {
        SVP_WARN("PlayPcmData(%p, %u) - g_pSimple[%p]!", pDate, nSize, g_pSimple);
        return false;
    }

    int32_t error = 0;

    //uint32_t tickTime = SVPTime::getTickCount();
    if (pa_simple_write(g_pSimple, pDate, nSize, &error) < 0) {
        SVP_ERROR("pa_simple_write() fail: %d", error);
        return false;
    }

    //uint32_t costTime = SVPTime::getTickCount()-tickTime;
    //SVP_INFO("pa_simple_write(%p, %p, %ld) - cost %ld ms.", g_pSimple, pDate, nSize, costTime);
    return true;
}

bool SVPPcmPlayer::PcmDrain()
{
    SVP_INFO_FUNC_BEGIN();
    if (g_pSimple)
    {
        int32_t error = 0;

        if (pa_simple_drain(g_pSimple, &error) < 0) {
            SVP_ERROR("pa_simple_drain() fail: %d", error);
        }
    }

    SVP_INFO_FUNC_END();
    return true;
}

void SVPPcmPlayer::Play(const std::string& fileName)
{
	uint8_t buf[FILE_BUF_LENGTH];
	uint32_t size;
	memset(buf, 0, FILE_BUF_LENGTH);
	FILE* pFile = fopen(fileName.c_str(), "r");
	if (pFile) {
		size = fread(buf, 1,  FILE_BUF_LENGTH, pFile);
		fclose(pFile);
		pFile = NULL;
	}
	if (g_pSimple) {
		SVP_INFO("Play File[%s]", fileName.c_str());
		PlayPcmData(buf, size);
//		usleep(KEY_DELAY_TIME);
		PcmDrain();
	}
}
