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

#include "SVPTime.h"

#include <time.h>

#include "SVPLog.h"

#ifdef SVP_LOG_TAG
    #undef SVP_LOG_TAG
#endif
#define SVP_LOG_TAG     "libbasic|time"

#if defined(SVP_WIN_32) || defined(SVP_WIN_CE)

#else
    #include <unistd.h>
    #include <sys/time.h>
    #include <netinet/in.h>
#endif

void SVPTime::getSystemTime(SVPTypeSystemTime& sys_time) {
#if defined(SVP_WIN_32) || defined(SVP_WIN_CE)
    ::GetSystemTime(&sys_time);
#else
    time_t curtm;
    time(&curtm);
    struct tm* ptm_sys = gmtime(&curtm);

    if (NULL == ptm_sys) {
        SVP_ERROR("getSystemTime() fail!");
        return;
    }

    sys_time.wYear          = ptm_sys->tm_year + 1900;
    sys_time.wMonth         = ptm_sys->tm_mon + 1;
    sys_time.wDayOfWeek     = ptm_sys->tm_wday;
    sys_time.wDay           = ptm_sys->tm_mday;
    sys_time.wHour          = ptm_sys->tm_hour;
    sys_time.wMinute        = ptm_sys->tm_min;
    sys_time.wSecond        = ptm_sys->tm_sec;
    sys_time.wMilliseconds  = 0;
    //printf("GET SYS: %s\n", asctime(ptm_sys));
#endif
}

void SVPTime::setSystemTime(const SVPTypeSystemTime& sys_time) {
#if defined(SVP_WIN_32) || defined(SVP_WIN_CE)
    ::SetSystemTime(&sys_time);
#else
    struct tm tm_sys;
    tm_sys.tm_year          = sys_time.wYear - 1900;
    tm_sys.tm_mon           = sys_time.wMonth - 1;
    tm_sys.tm_mday          = sys_time.wDay;
    tm_sys.tm_wday          = sys_time.wDayOfWeek;
    tm_sys.tm_hour          = sys_time.wHour;
    tm_sys.tm_min           = sys_time.wMinute;
    tm_sys.tm_sec           = sys_time.wSecond;
    // Correct any day of week and day of year etc. fields
    tm_sys.tm_isdst         = -1;
    //printf("SET SYS: %s\n", asctime(&tm_sys));
    time_t tt_sys = mktime(&tm_sys);

    if (tt_sys == -1) {
        SVP_ERROR("setSystemTime() - mktime() fail!");
        return;
    }

    tm* ptm_loc = gmtime(&tt_sys);

    if (NULL == ptm_loc) {
        SVP_ERROR("setSystemTime() - gmtime() fail!");
        return;
    }

    time_t tt_loc = (tt_sys - mktime(ptm_loc)) + tt_sys; // timzzone dist + sys time

    //SVP_INFO("SET LOC: %s", asctime(localtime(&tt_loc)));

    if (!stime(&tt_loc)) {
        // OK
        SVP_INFO("setSystemTime() - %04d-%02d-%02d %02d:%02d:%02d", sys_time.wYear, sys_time.wMonth, sys_time.wDay,
            sys_time.wHour, sys_time.wMinute, sys_time.wSecond );
    } else {
        // Failed
        SVP_ERROR("setSystemTime() - stime() fail!");
    }

#endif
}

void SVPTime::getLocalTime(SVPTypeSystemTime& loc_time) {
#if defined(SVP_WIN_32) || defined(SVP_WIN_CE)
    ::GetLocalTime(&loc_time);
#else
    time_t curtm;
    time(&curtm);
    //#if 0 // OS fixed time zone issue.
    //  curtm += 28800; //8*3600; 东8区
    //#endif
    tm* ptm_loc = localtime(&curtm);

    if (NULL == ptm_loc) {
        SVP_ERROR("getLocalTime() fail!");
        return;
    }

    loc_time.wYear          = ptm_loc->tm_year + 1900;
    loc_time.wMonth         = ptm_loc->tm_mon + 1;
    loc_time.wDayOfWeek     = ptm_loc->tm_wday;
    loc_time.wDay           = ptm_loc->tm_mday;
    loc_time.wHour          = ptm_loc->tm_hour;
    loc_time.wMinute        = ptm_loc->tm_min;
    loc_time.wSecond        = ptm_loc->tm_sec;
    loc_time.wMilliseconds  = 0;
#endif
}

void SVPTime::setLocalTime(const SVPTypeSystemTime& loc_time) {
#if defined(SVP_WIN_32) || defined(SVP_WIN_CE)
    ::SetLocalTime(&loc_time);
#else
    struct tm tm_loc;
    tm_loc.tm_year          = loc_time.wYear - 1900;
    tm_loc.tm_mon           = loc_time.wMonth - 1;
    tm_loc.tm_mday          = loc_time.wDay;
    tm_loc.tm_wday          = loc_time.wDayOfWeek;
    tm_loc.tm_hour          = loc_time.wHour;
    tm_loc.tm_min           = loc_time.wMinute;
    tm_loc.tm_sec           = loc_time.wSecond;
    // Correct any day of week and day of year etc. fields
    tm_loc.tm_isdst         = -1;
    time_t tt_loc = mktime(&tm_loc);

    if (tt_loc == -1) {
        SVP_ERROR("setLocalTime() - mktime() fail!");
        return;
    }
    //  tt_loc -= 28800; //8*3600; 东8区

    if (!stime(&tt_loc)) {
        // OK
        SVP_INFO("setLocalTime() - %04d-%02d-%02d %02d:%02d:%02d", loc_time.wYear, loc_time.wMonth, loc_time.wDay,
            loc_time.wHour, loc_time.wMinute, loc_time.wSecond );
    } else {
        // Failed
        SVP_ERROR("setLocalTime() - stime() fail!");
    }

#endif
}

uint32_t SVPTime::getTickCount() {
#if defined(SVP_WIN_32) || defined(SVP_WIN_CE)
    return ::GetTickCount();
#else
    //  struct timeval current;
    //  gettimeofday(&current, NULL);
    //  return (current.tv_sec * 1000 + current.tv_usec/1000);
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000 + ts.tv_nsec / (1000 * 1000));
#endif
}
