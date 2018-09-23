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

#include "SVPType.h"

/**
 * \brief Cross platform time class
 */

namespace SVPTime {
#if defined(SVP_WIN_32) || defined(SVP_WIN_CE)
/**
 * \brief define SVPTypeSystemTime as SYSTEMTIME on Win-based system
 */
typedef SYSTEMTIME    SVPTypeSystemTime;

#else
/**
 * \brief Structure of SVPTypeSystemTime
 */
struct SVPTypeSystemTime {
    uint16_t wYear;         /*!< Year (0 ~ ) */
    uint16_t wMonth;        /*!< Month (1 ~ 12) */
    uint16_t wDayOfWeek;    /*!< Day of week (0 ~ 6) */
    uint16_t wDay;          /*!< Day (1 ~ 31) */
    uint16_t wHour;         /*!< Hours (0 ~ 24) */
    uint16_t wMinute;       /*!< Minutes (0 ~ 59) */
    uint16_t wSecond;       /*!< Seconds (0 ~ 59) */
    uint16_t wMilliseconds; /*!< Millisecond (0) */
};
#endif

/**
 * \brief Get system (UTC) time
 * \param[in, out] sys_time system time
 */
void getSystemTime(SVPTypeSystemTime& sys_time);

/**
 * \brief Set system (UTC) time
 * \param[in] sys_time system time
 */
void setSystemTime(const SVPTypeSystemTime& sys_time);

/**
 * \brief Get local time ref to current timezone
 * \param[in, out] loc_time local time
 */
void getLocalTime(SVPTypeSystemTime& loc_time);

/**
 * \brief Set local time ref to current timezone
 * \param[in, out] loc_time local time
 */
void setLocalTime(const SVPTypeSystemTime& loc_time);

/**
 * \brief Get current tick count
 * \return Current tick count
 */
uint32_t getTickCount();
};

#define RUNTIME_COUNT(func) { \
        uint32_t ulTickCount = SVPTime::getTickCount(); \
        func;\
        ulTickCount = SVPTime::getTickCount() - ulTickCount; \
        printf("RUNTIME_COUNT: %s : %d ms\n", #func, ulTickCount); \
    }
