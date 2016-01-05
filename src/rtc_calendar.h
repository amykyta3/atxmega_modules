
#ifndef RTC_CALENDAR_H
#define RTC_CALENDAR_H

#include <stdint.h>

//==============================================================================
typedef struct{
    uint16_t year;      ///< Full year integer
    uint8_t month;      ///< Month (Jan=0, Feb=1, etc...)
    uint8_t day;        ///< Day of the month (1-31)
    uint8_t dayofweek;  ///< Day of the week (Sun=0, Mon=1, etc...)
    uint8_t hour;       ///< Hour in 24hr format (0-23)
    uint8_t minute;     ///< Minute (0-59)
    uint8_t second;     ///< Second (0-59)
}calendar_time_t;

#define UNKNOWN_DOW	0xFF ///< Use when \c dayofweek is not known.

typedef struct calendar_alarm_s calendar_alarm_t;
typedef struct{
    uint8_t dayofweek_mask;     ///< Bit-mask for which days alarm is active. (bit0=Sun...)
    uint8_t hour;               ///< Hour of alarm (0-23)
    uint8_t minute;             ///< Minute (0-59)
    void (*callback)(void*);    ///< User call-back function
    void *callback_data;        ///< User call-back data
    calendar_alarm_t *next;     ///< Reserved for internal use
}calendar_alarm_t;
//==============================================================================
/**
* \brief Initialize the RTC
* \note RTC clock source must already be configured and generating a 32.768 kHz clock.
**/
void calendar_init(void);

/**
* \brief Uninitialize the RTC
**/
void calendar_uninit(void);

/**
* \brief Sets the current calendar time
* \param [in] T Pointer to #time_t object
**/
void calendar_set_time(calendar_time_t *T);

/**
* \brief Sets the current Daylight Savings Time state
* \param Whether DST is observed in the current region
* \param Whether DST is currently set (summer hours?)
**/
void calendar_set_DST(uint8_t observed, uint8_t enabled);

/**
* \brief Gets the current calendar time
* \param [out] T Current time
**/
void calendar_get_time(calendar_time_t *T);

/**
* \brief Add an alarm
**/
void calendar_add_alarm(calendar_alarm_t *A);

/**
* \brief Add an alarm
**/
void calendar_remove_alarm(calendar_alarm_t *A);

#endif
