
#ifndef RTC_H
#define RTC_H

#include <stdint.h>
#include <stdbool.h>

#include <rtc_config.h>

//==============================================================================
// General RTC Functions
//==============================================================================
/**
* \brief Initialize the RTC
* \note RTC clock source must already be configured and generating a 32.768 kHz clock.
**/
void rtc_init(void);

/**
* \brief Uninitialize the RTC
**/
void rtc_uninit(void);

#if(RTC_CALENDAR_ENABLE)
//==============================================================================
// Calendar Types
//==============================================================================
typedef struct{
    uint16_t year;      ///< Full year integer
    uint8_t month;      ///< Month (Jan=0, Feb=1, etc...)
    uint8_t day;        ///< Day of the month (1-31)
    uint8_t dayofweek;  ///< Day of the week (Sun=0, Mon=1, etc...)
    uint8_t hour;       ///< Hour in 24hr format (0-23)
    uint8_t minute;     ///< Minute (0-59)
    uint8_t second;     ///< Second (0-59)
} calendar_time_t;

#define UNKNOWN_DOW	0xFF ///< Use when \c dayofweek is not known.

typedef struct calendar_alarm_s calendar_alarm_t;
typedef struct calendar_alarm_s{
    uint8_t dayofweek_mask;     ///< Bit-mask for which days alarm is active. (bit0=Sun...)
    uint8_t hour;               ///< Hour of alarm (0-23)
    uint8_t minute;             ///< Minute (0-59)
    void (*callback)(void*);    ///< User call-back function
    void *callback_data;        ///< User call-back data
    calendar_alarm_t *next;     ///< Reserved for internal use
} calendar_alarm_t;

//==============================================================================
// Calendar Functions
//==============================================================================
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

#endif // RTC_CALENDAR_ENABLE
//==============================================================================
// Event Timer Types
//==============================================================================
#if(RTC_TIMER_ENABLE)
/**
 * \brief Public structure used to define a new timer's behavior
 **/
struct timerctl{
    uint16_t interval;          ///< Timer interval in ticks
    bool repeat;                ///< Should the timer repeat? True or False
    void (*callback)(void*);    ///< Pointer to the function to call each time the timer expires
    void *callback_data;        ///< Pointer to a data object that will be passed into callback
};

// Timer object. User doesn't need to touch this.
typedef struct timer_s timer_t;
typedef struct timer_s{
    uint16_t ticks_remaining;   ///< Number of ticks remaining after previous timer. If first, relative to reftick
    uint16_t ticks_reload;      ///< When the timer expires, reload it. If 0, timer is a 1-shot
    void (*callback)(void*);    ///< User call-back function
    void *callback_data;        ///< User call-back data
    timer_t *next;              ///< Reserved for internal use
} timer_t;

//==============================================================================
// Event Timer Functions
//==============================================================================
/**
 * \brief Creates a new or resumes an existing interval timer.
 * 
 * The new timer object is returned in the buffer pointed to by \c timerid, which must be a non-NULL
 * pointer.  This timer object can not be deallocated until after the timer has been stopped.
 * 
 * The \c settings argument points to a \ref timerctl structure that specifies how the timer
 * operates. A previously stopped timer can be resumed by passing a NULL pointer into the 
 * \c settings argument.
 * 
 * \param timerid Pointer to the timer object
 * \param settings Pointer to a \ref timerctl struct which defines the behavior of a new timer.
 *     A NULL pointer will resume a previously stopped timer defined by \c timerid
 **/
void timer_start(timer_t *timerid, struct timerctl *settings);

/**
 * \brief Stops a currently running timer
 * 
 * When stopping a timer, the state of the timer count is preserved allowing it to be resumed at a
 * later time. A stopped timer can be resumed by passing it into timer_start() along with a NULL
 * pointer in place of the \c settings argument
 * 
 * \param timerid Pointer to the timer object to stop
 **/
void timer_stop(timer_t *timerid);

#endif // RTC_TIMER_ENABLE

#endif
