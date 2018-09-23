#ifndef  SVP_PRO_CONFIG_H_
#define  SVP_PRO_CONFIG_H_

#include <stdio.h>
#include <string.h>

#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#define __PID__         getpid()
#define __TID__         pthread_self()

#define SVP_LOG_TAG         "SVP"

//#define SVP_NO_DEBUG
//#define SVP_NO_INFO
//#define SVP_NO_WARN
//#define SVP_NO_ERROR

#ifdef SVP_NO_DEBUG
#define SVP_DEBUG(argfmt, ...)
#else
#define SVP_DEBUG(argfmt, ...) \
    {\
        printf("(%u|0x%08lX) [%s] DEBUG: " argfmt "\r\n", __PID__, __TID__, SVP_LOG_TAG, ##__VA_ARGS__);\
    }
#endif


#ifdef SVP_NO_INFO
#define SVP_INFO(argfmt, ...)
#else
#define SVP_INFO(argfmt, ...) \
    {\
        printf("(%u|0x%08lX) [%s] INFO: " argfmt "\r\n", __PID__, __TID__, SVP_LOG_TAG, ##__VA_ARGS__);\
    }
#endif

#ifdef SVP_NO_WARN
#define SVP_WARN(argfmt, ...)
#else
#define SVP_WARN(argfmt, ...) \
    {\
        printf("(%u|0x%08lX) [%s] WARN: " argfmt "\r\n", __PID__, __TID__, SVP_LOG_TAG, ##__VA_ARGS__);\
    }
#endif

#ifdef SVP_NO_ERROR
#define SVP_ERROR(argfmt, ...)
#else
#define SVP_ERROR(argfmt, ...) \
    {\
        printf("(%u|0x%08lX) [%s] ERROR: " argfmt "\r\n", __PID__, __TID__, SVP_LOG_TAG, ##__VA_ARGS__);\
    }
#endif

// Extentions
#ifdef SVP_NO_INFO
#define SVP_INFO_FUNC()
#else
#define SVP_INFO_FUNC() \
    {\
        SVP_INFO("%s()", __FUNCTION__);\
    }
#endif

#ifdef SVP_NO_INFO
#define SVP_INFO_FUNC_BEGIN()
#else
#define SVP_INFO_FUNC_BEGIN() \
    {\
        SVP_INFO("%s() - Begin.", __FUNCTION__);\
    }
#endif

#ifdef SVP_NO_INFO
#define SVP_INFO_FUNC_END()
#else
#define SVP_INFO_FUNC_END() \
    {\
        SVP_INFO("%s() - End", __FUNCTION__);\
    }
#endif

#ifdef SVP_NO_INFO
#define SVP_INFO_BUILD_TIME()
#else
#define SVP_INFO_BUILD_TIME() \
    {\
        SVP_INFO("Build Time -- %s %s.", __DATE__, __TIME__);\
    }
#endif

#endif
