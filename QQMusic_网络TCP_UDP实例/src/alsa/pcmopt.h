#ifndef SVP_PCM_OPT_H_
#define SVP_PCM_OPT_H_
#include <alsa/asoundlib.h>

class pcmopt
{
public:
	typedef enum
	{
	    VUMETER_NONE,
	    VUMETER_MONO,
	    VUMETER_STEREO,
	}METER_TYPE;

	struct HWPARAMS
	{
	    snd_pcm_format_t format;    // 格式，类型
	    unsigned int channels;      // 几通道
	    unsigned int rate;          // 采样频率
	};

public:
	void prg_exit(int code);
	bool opendevice();
	ssize_t pcm_write(char *data, size_t count);
	void compute_max_peak(void *data, size_t count);
	void print_vu_meter(signed int *perc, signed int *maxperc); //parasoft-suppress MISRA2008-2_10_5_a
	void suspend();
	void do_pause();
	void xrun();
	void set_params();
	/* needed prototypes */
	void init_stdin();
	void done_stdin();
	void check_stdin();
private:
	ssize_t safe_read(int handle_fd, void *buf, size_t count);
	void print_vu_meter_mono(int perc, int maxperc); //parasoft-suppress MISRA2008-2_10_5_a
	void print_vu_meter_stereo(int *perc, int *maxperc); //parasoft-suppress MISRA2008-2_10_5_a

private:
	snd_pcm_sframes_t (*readi_func)(snd_pcm_t *handle, void *buffer, snd_pcm_uframes_t size); 			//parasoft-suppress INIT-04 "不做修改"
	snd_pcm_sframes_t (*writei_func)(snd_pcm_t *handle, const void *buffer, snd_pcm_uframes_t size);	//parasoft-suppress INIT-04 "不做修改"
	snd_pcm_sframes_t (*readn_func)(snd_pcm_t *handle, void **bufs, snd_pcm_uframes_t size);			//parasoft-suppress INIT-04 "不做修改"
	snd_pcm_sframes_t (*writen_func)(snd_pcm_t *handle, void **bufs, snd_pcm_uframes_t size);			//parasoft-suppress INIT-04 "不做修改"
	snd_pcm_t *handle = NULL; //parasoft-suppress INIT-04 "不做修改"
	HWPARAMS hwparams;

	 int quiet_mode = 0;
	 snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;          //SND_PCM_STREAM_PLAYBACK：Playback stream； SND_PCM_STREAM_CAPTURE：Capture stream
	 snd_pcm_uframes_t chunk_size = 0;
	 unsigned period_time = 0;
	 unsigned buffer_time = 0;
	 snd_pcm_uframes_t period_frames = 0;
	 snd_pcm_uframes_t buffer_frames = 0;
	 int avail_min = -1;
	 int start_delay = 0;
	 int stop_delay = 0;
	 int monotonic = 0;
	 int can_pause = 0;

	 int vumeter = VUMETER_NONE;
	 size_t bits_per_sample, bits_per_frame;
	 size_t chunk_bytes;
	 int test_nowait = 0;
	 snd_output_t *log; //parasoft-suppress INIT-04 "不做修改"
	 long term_c_lflag = -1;

	//计算每秒的数据，及进度条用
	 float CoPerSec;
	 int Counter = 0;
	 int bytesperget = 0;

//	typedef void(*signal_handler)(int);

};

#ifndef timersub
#define	timersub(a, b, result) \
do { \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec; \
    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
    if ((result)->tv_usec < 0) { \
        --(result)->tv_sec; \
        (result)->tv_usec += 1000000; \
    } \
}while(0)
#endif



#ifndef timermsub
#define	timermsub(a, b, result) \
do { \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec; \
    (result)->tv_nsec = (a)->tv_nsec - (b)->tv_nsec; \
    if ((result)->tv_nsec < 0) { \
        --(result)->tv_sec; \
        (result)->tv_nsec += 1000000000L; \
    } \
}while(0)
#endif

#endif
