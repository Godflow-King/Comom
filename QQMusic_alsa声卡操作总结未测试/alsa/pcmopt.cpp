#include "alsa/pcmopt.h"
#include "alsa/aconfig.h"
#include "alsa/gettext.h"
#include "alsa/formats.h"
#include "config.h"

#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <locale.h>
#include <assert.h>
#include <termios.h>
#include <sys/poll.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/signal.h>
#include <asm/byteorder.h>

#define aset

volatile static int recycle_capture_file = 0;

void pcmopt::prg_exit(int code)
{
    SVP_INFO("qqmusic error prg_exit(%d);",code);
    if (handle)
    {
        snd_pcm_close(handle);
        handle = NULL;
    }
}

/*
 * Safe read (for pipes)
 */
ssize_t pcmopt::safe_read(int handle_fd, void *buf, size_t count)
{
    ssize_t result = 0, res;
    while (count > 0)
    {
        if ((res = read(handle_fd, buf, count)) == 0)
            break;
        if (res < 0)
            return result > 0 ? result : res;
        count -= res;
        result += res;
        buf = (char *)buf + res;
    }
    return result;
}

/*
 */

void pcmopt::set_params()
{
    snd_pcm_hw_params_t *params; //parasoft-suppress INIT-04 "不做修改"
    snd_pcm_sw_params_t *swparams; //parasoft-suppress INIT-04 "不做修改"
    snd_pcm_uframes_t buffer_size;
    int err;
    size_t n;
    unsigned int rate;
    snd_pcm_uframes_t start_threshold, stop_threshold;

    SVP_INFO("qqmusic hwparams.format is %d,hwparams.rate is %d,hwparams.channels is %d",
           hwparams.format,hwparams.rate,hwparams.channels);
resetpcm:
    buffer_time = 0 ;
    buffer_frames = 0;
    period_time = 0;
    period_frames = 0;

    //0. 关闭PCM设备句柄
    snd_pcm_close(handle);

    //打开句柄
    err = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    while (err < 0)
    {
    	SVP_INFO("qqmusic error snd_pcm_open");
        err = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    }


    snd_pcm_hw_params_alloca(&params);
    snd_pcm_sw_params_alloca(&swparams);

    err = snd_pcm_hw_params_any(handle, params);
    if (err < 0) {
        SVP_INFO("qqmusic error prg_exit is due to set_params: res = snd_pcm_hw_params_any(handle, params)<0.");
        goto resetpcm;  //parasoft-suppress MISRA2008-6_6_2
        //prg_exit(EXIT_FAILURE);
    }

    err = snd_pcm_hw_params_set_access(handle, params,
                           SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err < 0) {
        SVP_INFO("qqmusic error prg_exit is due to set_params: snd_pcm_hw_params_set_access(handle, params,SND_PCM_ACCESS_RW_INTERLEAVED)<0.\n");
        goto resetpcm; //parasoft-suppress MISRA2008-6_6_2
    }

    //设置格式，这里比较容易失败
    err = snd_pcm_hw_params_set_format(handle, params, hwparams.format);
    if (err < 0) {//再尝试一次
        err = snd_pcm_hw_params_set_format(handle, params, hwparams.format);
        if (err < 0) {//第二次尝试
            err = snd_pcm_hw_params_set_format(handle, params, hwparams.format);
            if (err < 0) {
                SVP_INFO("qqmusic error prg_exit is due to set_params: res = snd_pcm_hw_params_set_format(handle, params, %d)<0.\n",hwparams.format);
                goto resetpcm; //parasoft-suppress MISRA2008-6_6_2
                //prg_exit(EXIT_FAILURE);
            }
        }
    }

    SVP_INFO("qqmusic buffer_time is %d; \n",buffer_time);

    //设置通道
    err = snd_pcm_hw_params_set_channels(handle, params, hwparams.channels);
    if (err < 0) {
        SVP_INFO("qqmusic error  snd_pcm_hw_params_set_channels(handle, params, hwparams.channels)<0.\n");
        hwparams.channels = 2;
        err = snd_pcm_hw_params_set_channels(handle, params, hwparams.channels);
        if (err < 0) {
            SVP_INFO("snd_pcm_hw_params_set_channels(handle, params, hwparams.channels)<0.\n");
            goto resetpcm; //parasoft-suppress MISRA2008-6_6_2
        }
    }

    rate = hwparams.rate;

    //设置采样频率
    err = snd_pcm_hw_params_set_rate_near(handle, params, &hwparams.rate, 0);
    assert(err >= 0);

    if ((float)rate * 1.05 < hwparams.rate || (float)rate * 0.95 > hwparams.rate)
    {
        if (!quiet_mode)
        {
            char plugex[64];
            const char *pcmname = snd_pcm_name(handle);
            fprintf(stderr, _("qqmusic Warning: rate is not accurate (requested = %iHz, got = %iHz)\n"), rate, hwparams.rate);
            if (! pcmname || strchr(snd_pcm_name(handle), ':'))
                *plugex = 0;
            else
                snprintf(plugex, sizeof(plugex), "(-Dplug:%s)",
                snd_pcm_name(handle));
                fprintf(stderr, _("qqmusic please, try the plug plugin %s\n"),
                plugex);
        }
    }
    rate = hwparams.rate;

#ifdef aset

    if (buffer_time == 0 && buffer_frames == 0)
    {
        err = snd_pcm_hw_params_get_buffer_time_max(params, &buffer_time, 0);
        assert(err >= 0);

        if (buffer_time > 500000)// 这里本来是500000
        {
            buffer_time = 500000;
        }
        else//自己添加
        {
            buffer_time = buffer_time/200000 * 200000;
        }
        SVP_INFO("qqmusic alsa buffer_time is %d; \n",buffer_time);
    }

    if (period_time == 0 && period_frames == 0) {
        if (buffer_time > 0)
            period_time = buffer_time / 4;//这里本来是4
        else
            period_frames = buffer_frames / 4;
    }
    SVP_INFO("qqmusic alsa period_time is %d; \n",period_time);

    if (period_time > 0)
        err = snd_pcm_hw_params_set_period_time_near(handle, params,&period_time, 0);
    else
        err = snd_pcm_hw_params_set_period_size_near(handle, params,&period_frames, 0);
    assert(err >= 0);


    if (buffer_time > 0)
    {
        err = snd_pcm_hw_params_set_buffer_time_near(handle, params,&buffer_time, 0);
    } else
    {
        err = snd_pcm_hw_params_set_buffer_size_near(handle, params,&buffer_frames);
    }
    assert(err >= 0);
    SVP_INFO("qqmusic buffer_frames is %ld; \n",buffer_frames);

#else

    err = snd_pcm_hw_params_get_buffer_size_max(params, &buffer_size);

    assert(err >= 0);

    if (buffer_size > 100000)// 这里本来是500000
    {
        buffer_size = 100000;
    }
    else//自己添加
    {
        buffer_size = buffer_size/20000 * 20000;
    }
    SVP_INFO("qqmusic alsa buffer_size is %d; \n", buffer_size);

    period_frames = buffer_size/10;

    err = snd_pcm_hw_params_set_period_size_near(handle, params,&period_frames, 0);
    assert(err >= 0);

    err = snd_pcm_hw_params_set_buffer_size_near(handle, params,&buffer_size);
    assert(err >= 0);

    SVP_INFO("qqmusic period_frames is %ld; \n",period_frames);
#endif



    //Check if timestamps are monotonic for given configuration.
    //This function should only be called when the configuration space contains a single configuration.
    monotonic = snd_pcm_hw_params_is_monotonic(params);

    //cheke if the hardware support pause
    can_pause = snd_pcm_hw_params_can_pause(params);
    err = snd_pcm_hw_params(handle, params);
    if (err < 0)
    {
        snd_pcm_hw_params_dump(params, log);
        SVP_INFO("qqmusic error prg_exit is due to set_params: res = snd_pcm_hw_params(handle, params)<0.\n");
        prg_exit(EXIT_FAILURE);
    }

    //get the chunk_size and buffer_size setting from hardware
    snd_pcm_hw_params_get_period_size(params, &chunk_size, 0);
    snd_pcm_hw_params_get_buffer_size(params, &buffer_size);

    SVP_INFO("qqmusic alsa period_size and chunk_size is %ld; \n",chunk_size);
    SVP_INFO("qqmusic alsa buffer_size is %ld; \n",buffer_size);

    if (chunk_size == buffer_size) {
        SVP_INFO("qqmusic error prg_exit is due to set_params: chunk_size == buffer_size.\n");
        prg_exit(EXIT_FAILURE);
    }

    snd_pcm_sw_params_current(handle, swparams);
    if (avail_min < 0)
        n = chunk_size;
    else
        n = (double) rate * avail_min / 1000000;
    err = snd_pcm_sw_params_set_avail_min(handle, swparams, n);//这里原本是n


/////阀值设置，这个很关键的
    /* round up to closest transfer boundary */
    n = buffer_size;
    if (start_delay <= 0) {
        start_threshold = n + (double) rate * start_delay / 1000000;
    } else
        start_threshold = (double) rate * start_delay / 1000000;
    if (start_threshold < 1)
        start_threshold = 1;
    if (start_threshold > n)
        start_threshold = n;

   //PCM is automatically started when playback frames available to PCM are >= threshold or when requested capture frames are >= threshold
    err = snd_pcm_sw_params_set_start_threshold(handle, swparams, start_threshold);//本来是start_threshold
    assert(err >= 0);
    SVP_INFO("qqmusic alsa start_threshold is %ld; \n",start_threshold);//本来是2

    //PCM is automatically stopped in SND_PCM_STATE_XRUN state when available frames is >= threshold. If the stop threshold is equal to
    //boundary (also software parameter - sw_param) then automatic stop will be disabled (thus device will do the endless loop in the ring buffer).
    if (stop_delay <= 0)
        stop_threshold = buffer_size + (double) rate * stop_delay / 1000000;
    else
        stop_threshold = (double) rate * stop_delay / 1000000;

    err = snd_pcm_sw_params_set_stop_threshold(handle, swparams, stop_threshold);
    assert(err >= 0);

    if (snd_pcm_sw_params(handle, swparams) < 0) {
        snd_pcm_sw_params_dump(swparams, log);
        SVP_INFO("qqmusic error prg_exit is due to set_params: snd_pcm_sw_params(handle, swparams) < 0.\n");
        prg_exit(EXIT_FAILURE);
    }
/////阀值设置


    //计算audiobuf的长度
    bits_per_sample = snd_pcm_format_physical_width(hwparams.format);
    SVP_INFO("qqmusic bits_per_sample is %d; ",bits_per_sample);

    bits_per_frame = bits_per_sample * hwparams.channels;
    SVP_INFO("qqmusic bits_per_frame is %d;",bits_per_frame);

    // 采样n（chunk_size）次需要的数据量
    chunk_bytes = chunk_size * bits_per_frame / 8;
    SVP_INFO("qqmusic chunk_bytes is %d;",chunk_bytes);

#ifdef USERAUDIOBUF
//    audiobuf = realloc(audiobuf, chunk_bytes);
//    if (audiobuf == NULL) {
//        SVP_INFO("qqmusic error prg_exit is due to set_params: audiobuf == NULL.\n");
//        //prg_exit(EXIT_FAILURE);
//    }
#endif
    /* stereo VU-meter isn't always available... */

    if (vumeter == VUMETER_STEREO) {
        if (hwparams.channels != 2)
            vumeter = VUMETER_MONO;
    }
    buffer_frames = buffer_size;	/* for position test */

    SVP_INFO("qqmusic end of set_params\n");
}

void pcmopt::init_stdin()
{
    struct termios term;
    long flags;
    tcgetattr(fileno(stdin), &term);
    term_c_lflag = term.c_lflag;
    if (-1 == fileno(stdin))
        return;
    flags = fcntl(fileno(stdin), F_GETFL);
    if (flags < 0 || fcntl(fileno(stdin), F_SETFL, flags|O_NONBLOCK) < 0)
        fprintf(stderr, _("qqmusic stdin O_NONBLOCK flag setup failed\n"));
    term.c_lflag &= ~ICANON;
    tcsetattr(fileno(stdin), TCSANOW, &term);
}

void pcmopt::done_stdin()
{
    struct termios term;
    if (-1 == fileno(stdin) || term_c_lflag == -1)
        return;
    tcgetattr(fileno(stdin), &term);
    term.c_lflag = term_c_lflag;
    tcsetattr(fileno(stdin), TCSANOW, &term);
}

void pcmopt::check_stdin()
{
    unsigned char b;

    if (-1 != fileno(stdin))
    {
        while (read(fileno(stdin), &b, 1) == 1)
        {
            if (b == ' ' || b == '\r')
            {
                while (read(fileno(stdin), &b, 1) == 1); //parasoft-suppress MISRA2008-6_3_1
                fprintf(stderr, _("qqmusic \r=== PAUSE ===                                                            "));
                fflush(stderr);
                do_pause();
                fprintf(stderr, "qqmusic                                                                          \r");
                fflush(stderr);
            }
        }
    }
}


/* I/O error handler */
void pcmopt::xrun()
{
    snd_pcm_status_t *status = NULL;
    int res;
    snd_pcm_status_alloca(&status);

    if ((res = snd_pcm_status(handle, status))<0) {
        SVP_INFO("qqmusic error prg_exit is due to xrun:snd_pcm_status(handle, status))<0\n");
        prg_exit(EXIT_FAILURE);
    }
    if (snd_pcm_status_get_state(status) == SND_PCM_STATE_XRUN) {
        if (monotonic) {
#ifdef HAVE_CLOCK_GETTIME
            struct timespec now, diff, tstamp;
            clock_gettime(CLOCK_MONOTONIC, &now);
            snd_pcm_status_get_trigger_htstamp(status, &tstamp);
            timermsub(&now, &tstamp, &diff);
            fprintf(stderr, _("qqmusic %s!!! (at least %.3f ms long)\n"),
                stream == SND_PCM_STREAM_PLAYBACK ? _("underrun") : _("overrun"),
                diff.tv_sec * 1000 + diff.tv_nsec / 10000000.0);
#else
            fprintf(stderr, "qqmusic %s !!!\n", _("underrun"));
#endif

        } else {
            struct timeval now, diff, tstamp;
            gettimeofday(&now, 0);
            snd_pcm_status_get_trigger_tstamp(status, &tstamp);
            timersub(&now, &tstamp, &diff);
            fprintf(stderr, _("qqmusic %s!!! (at least %.3f ms long)\n"),
                stream == SND_PCM_STREAM_PLAYBACK ? _("underrun") : _("overrun"),
                diff.tv_sec * 1000 + diff.tv_usec / 1000.0);
        }

        if ((res = snd_pcm_prepare(handle))<0) {
            //error(_("xrun: prepare error: %s"), snd_strerror(res));
            SVP_INFO("qqmusic error prg_exit is due to xrun: res = snd_pcm_prepare(handle))<0(1).\n");
            prg_exit(EXIT_FAILURE);
        }
        return;		/* ok, data should be accepted again */
    }
    if (snd_pcm_status_get_state(status) == SND_PCM_STATE_DRAINING)
    {

        if (stream == SND_PCM_STREAM_CAPTURE) {
            fprintf(stderr, _("qqmusic capture stream format change? attempting recover...\n"));
            if ((res = snd_pcm_prepare(handle))<0) {
                //error(_("xrun(DRAINING): prepare error: %s"), snd_strerror(res));
                SVP_INFO("qqmusic error prg_exit is due to xrun: res = snd_pcm_prepare(handle))<0(2).\n");
                prg_exit(EXIT_FAILURE);
            }
            return;
        }
    }
    SVP_INFO("qqmusic error prg_exit is due to xrun: end of xrun.\n");
    prg_exit(EXIT_FAILURE);
}

void pcmopt::do_pause()
{
    int err;
    unsigned char b;
    if (!can_pause) {
        fprintf(stderr, _("qqmusic \rPAUSE command ignored (no hw support)\n"));
        return;
    }
    err = snd_pcm_pause(handle, 1);
    if (err < 0) {
        //error(_("pause push error: %s"), snd_strerror(err));
        return;
    }
    while (1) {
        while (read(fileno(stdin), &b, 1) != 1); //parasoft-suppress MISRA2008-6_3_1
        if (b == ' ' || b == '\r') {
            while (read(fileno(stdin), &b, 1) == 1); //parasoft-suppress MISRA2008-6_3_1
            err = snd_pcm_pause(handle, 0);
            if (err < 0)
            return;
        }
    }
}

/* I/O suspend handler */

void pcmopt::suspend()
{
    int res;
    if (!quiet_mode)
        fprintf(stderr, _("qqmusic Suspended. Trying resume. ")); fflush(stderr);
    while ((res = snd_pcm_resume(handle)) == -EAGAIN) //parasoft-suppress MISRA2008-6_3_1
    {
        sleep(1);	/* wait until suspend flag is released */
    }
    if (res < 0) {
        if (!quiet_mode)
            fprintf(stderr, _("qqmusic Failed. Restarting stream. ")); fflush(stderr);
        if ((res = snd_pcm_prepare(handle)) < 0) {
            //error(_("suspend: prepare error: %s"), snd_strerror(res));
            SVP_INFO("qqmusic error prg_exit is due to suspend: res = snd_pcm_prepare(handle))<0.\n");
            prg_exit(EXIT_FAILURE);
        }
    }
    if (!quiet_mode)
        fprintf(stderr, _("qqmusic Done.\n"));
}

void pcmopt::print_vu_meter_mono(int perc, int maxperc) //parasoft-suppress MISRA2008-2_10_5_a
{
    const int bar_length = 50;
    char line[80];
    int val;

    for (val = 0; val <= perc * bar_length / 100 && val < bar_length; val++)
    {
        line[val] = '#';
    }
    for (; val <= maxperc * bar_length / 100 && val < bar_length; val++)
    {
        line[val] = ' ';
    }
    line[val] = '+';
    for (++val; val <= bar_length; val++)
    {
        line[val] = ' ';
    }
    if (maxperc > 99)
        sprintf(line + val, "| MAX");
    else
        sprintf(line + val, "| %02i%%", maxperc);
    fputs(line, stdout);
    if (perc > 100)
       printf(_(" !clip  "));
}

void pcmopt::print_vu_meter_stereo(int *perc, int *maxperc) //parasoft-suppress MISRA2008-2_10_5_a
{
    const int bar_length = 35;
    char line[80];
    int c;

    memset(line, ' ', sizeof(line) - 1);
    line[bar_length + 3] = '|';

    for (c = 0; c < 2; c++) {
        int p = perc[c] * bar_length / 100;
        char tmp[4];
        if (p > bar_length)
            p = bar_length;
        if (c)
            memset(line + bar_length + 6 + 1, '#', p);
        else
            memset(line + bar_length - p - 1, '#', p);
        p = maxperc[c] * bar_length / 100;
        if (p > bar_length)
            p = bar_length;
        if (c)
            line[bar_length + 6 + 1 + p] = '+';
        else
            line[bar_length - p - 1] = '+';//parasoft-suppress BD-PB-ARRAY "有可能越界,暂时不改"
        if (maxperc[c] > 99)
            sprintf(tmp, "MAX");
        else
            sprintf(tmp, "%02d%%", maxperc[c]);
        if (c)
            memcpy(line + bar_length + 3 + 1, tmp, 3);
        else
            memcpy(line + bar_length, tmp, 3);
    }
    line[bar_length * 2 + 6 + 2] = 0;
    fputs(line, stdout);
}

void pcmopt::print_vu_meter(signed int *perc, signed int *maxperc) //parasoft-suppress MISRA2008-2_10_5_a
{
    if (vumeter == VUMETER_STEREO)
        print_vu_meter_stereo(perc, maxperc);
    else
        print_vu_meter_mono(*perc, *maxperc);
}


/* peak handler */
void pcmopt::compute_max_peak(void *data, size_t count)
{
    signed int val, max, perc[2], max_peak[2];
    static	int	run = 0;
    int	format_little_endian = snd_pcm_format_little_endian(hwparams.format);
    int ichans, c;

    if (vumeter == VUMETER_STEREO)
        ichans = 2;
    else
        ichans = 1;

    memset(max_peak, 0, sizeof(max_peak));
    switch (bits_per_sample)
    {
    case 8: {
        signed char *valp = (signed char *)data;
        signed char mask = snd_pcm_format_silence(hwparams.format);
        c = 0;
        while (count-- > 0) {
            val = *valp++ ^ mask; //parasoft-suppress MISRA2008-5_0_20
            val = abs(val);
            if (max_peak[c] < val)
                max_peak[c] = val;
            if (vumeter == VUMETER_STEREO)
                c = !c;
        }
        break;
    }
    case 16: {
        signed short *valp = (signed short *)data;
        signed short mask = snd_pcm_format_silence_16(hwparams.format);
        signed short sval;

        count /= 2;
        c = 0;
        while (count-- > 0) {
            if (0 != format_little_endian)
                sval = __le16_to_cpu(*valp);
            else
                sval = __be16_to_cpu(*valp); //parasoft-suppress MISRA2008-5_0_14 PB-29
            sval = abs(sval) ^ mask; //parasoft-suppress MISRA2008-5_0_20
            if (max_peak[c] < sval)
                max_peak[c] = sval;
            valp++;
            if (vumeter == VUMETER_STEREO)
                c = !c;
        }
        break;
    }
    case 24: {
        unsigned char *valp =  (unsigned char*)data;
        signed int mask = snd_pcm_format_silence_32(hwparams.format);
        count /= 3;
        c = 0;
        while (count-- > 0) {
            if (format_little_endian) {
                val = valp[0] | (valp[1]<<8) | (valp[2]<<16);
            } else {
                val = (valp[0]<<16) | (valp[1]<<8) | valp[2];
            }

            /* Correct signed bit in 32-bit value */
            if (val & (1<<(bits_per_sample-1))) {
                val |= 0xff<<24;	/* Negate upper bits too */
            }
            val = abs(val) ^ mask;
            if (max_peak[c] < val)
                max_peak[c] = val;
            valp += 3;
            if (vumeter == VUMETER_STEREO)
                c = !c;
        }
        break;
    }
    case 32: {
        signed int *valp = (signed int *)data;
        signed int mask = snd_pcm_format_silence_32(hwparams.format);
        count /= 4;
        c = 0;
        while (count-- > 0) {
            if (0 != format_little_endian)
                val = __le32_to_cpu(*valp);
            else
                val = __be32_to_cpu(*valp); //parasoft-suppress MISRA2008-5_0_14
            val = abs(val) ^ mask;
            if (max_peak[c] < val)
                max_peak[c] = val;
            valp++;
            if (vumeter == VUMETER_STEREO)
                c = !c;
        }
        break;
    }
    default:
        if (run == 0)
        {
            fprintf(stderr, _("qqmusic Unsupported bit size %d.\n"), (int)bits_per_sample);
            run = 1;
        }
        return;
        break;
    }
    max = 1 << (bits_per_sample-1);
    if (max <= 0)
        max = 0x7fffffff;

    for (c = 0; c < ichans; c++) {
        if (bits_per_sample > 16)
            perc[c] = max_peak[c] / (max / 100);
        else
            perc[c] = max_peak[c] * 100 / max;
    }


    static int maxperc[2];
    static time_t t=0;
    const time_t tt=time(NULL);
    if(tt>t) {
        t=tt;
        maxperc[0] = 0;
        maxperc[1] = 0;
    }

    for (c = 0; c < ichans; c++)
    {
        if (perc[c] > maxperc[c])
        {
            maxperc[c] = perc[c];
        }
    }
    putchar('\r');
    print_vu_meter(perc, maxperc);
    fflush(stdout);
}


/*
 *  write function
 */
//写数据到PCM，具体大小为chunk_size，所以要清楚设计chunk_size的大小，具体到时候就调用这个函数
ssize_t pcmopt::pcm_write(char *data, size_t count)
{
    //SVP_INFO("qqmusic qqmusic pcm_write(1)");
    ssize_t r;
    ssize_t result = 0;
    if (count < chunk_size) {
        snd_pcm_format_set_silence(hwparams.format, data + count * bits_per_frame / 8, (chunk_size - count) * hwparams.channels);
        count = chunk_size;
    }
    //SVP_INFO("qqmusic pcm_write(2)");
    while (count > 0)
    {
        //check_stdin();// 非播放文件不用启动
        r = writei_func(handle, data, count);
        if (r == -EAGAIN || (r >= 0 && (size_t)r < count))
        {
            if (!test_nowait)
                snd_pcm_wait(handle, 100);
        }
        else if (r == -EPIPE)
        {
            SVP_INFO("qqmusic into xrun()");
            xrun();
            //完成硬件参数设置，使设备准备好
            SVP_INFO("qqmusic end xrun()");
        }
        else if (r == -ESTRPIPE)
        {
            SVP_INFO("qqmusic into suspend()");
            suspend();
            SVP_INFO("qqmusic end suspend()");
        }
        else if (r < 0)
        {
            SVP_INFO("qqmusic error prg_exit is due to pcm_write(): (-EAGAIN || (r >= 0 && (size_t)r < count) <0.\n");
            //prg_exit(EXIT_FAILURE);
            SVP_INFO("writing pcm data to alsa,but source isn't in qqmusic");
            return 1;
        }
        //SVP_INFO("qqmusic pcm_write(3)");
        if (r > 0)
        {
            if (vumeter)
                compute_max_peak(data, r * hwparams.channels);
                //SVP_INFO("qqmusic pcm_write(4)");
            result += r;
            count -= r;
            data += r * bits_per_frame / 8;
        }
    }
    return result;
}

bool pcmopt::opendevice()
{
    int  ret;
    //1. 打开PCM，最后一个参数为0意味着标准配置
    ret = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (ret < 0)
    {
        SVP_INFO("qqmusic error snd_pcm_open");
        //尝试去执行打开alsa的句柄
        for(int i = 0; snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0; i++) //parasoft-suppress MISRA2008-6_5_1 "暂不修改"
        {
            if(i > 10)
            {
            	SVP_INFO("can't not open the alsa handle.");
                return false;
            }
            sleep(1);
        }
    }

    writei_func = snd_pcm_writei;
    readi_func = snd_pcm_readi;
    writen_func = snd_pcm_writen;
    readn_func = snd_pcm_readn;
    chunk_size = 1024;

    //
    hwparams.format = SND_PCM_FORMAT_S16_LE;
    hwparams.channels = 2;
    hwparams.rate = 44100;
    return true;
}
