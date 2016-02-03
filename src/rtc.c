
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include "rtc.h"

//------------------------------------------------------------------------------

#if(RTC_CLKSRC_gc == CLK_RTCSRC_ULP_gc)         // 1 kHz from internal 32kHz ULP
    #define RTC_CLK_FREQ 1000UL
#elif(RTC_CLKSRC_gc == CLK_RTCSRC_TOSC_gc)      // 1.024 kHz from 32.768 kHz crystal oscillator on TOSC
    #define RTC_CLK_FREQ 1024UL
#elif(RTC_CLKSRC_gc == CLK_RTCSRC_RCOSC_gc)     // 1.024 kHz from 32.768 kHz internal oscillator
    #define RTC_CLK_FREQ 1024UL
#elif(RTC_CLKSRC_gc == CLK_RTCSRC_TOSC32_gc)    // 32.768 kHz from 32.768 kHz crystal oscillator on TOSC
    #define RTC_CLK_FREQ 32768UL
#elif(RTC_CLKSRC_gc == CLK_RTCSRC_RCOSC32_gc)   // 32.768 kHz from 32.768 kHz internal oscillator
    #define RTC_CLK_FREQ 32768UL
#elif(RTC_CLKSRC_gc == CLK_RTCSRC_EXTCLK_gc)    // External Clock from TOSC1
    #error "CLK_RTCSRC_EXTCLK_gc is not supported"
#else
    #error "Invalid RTC_CLKSRC_gc"
#endif

#if(RTC_PRESCALER_gc == RTC_PRESCALER_DIV1_gc)
    #define RTC_PRESCALER   1
#elif(RTC_PRESCALER_gc == RTC_PRESCALER_DIV2_gc)
    #define RTC_PRESCALER   2
#elif(RTC_PRESCALER_gc == RTC_PRESCALER_DIV8_gc)
    #define RTC_PRESCALER   8
#elif(RTC_PRESCALER_gc == RTC_PRESCALER_DIV16_gc)
    #define RTC_PRESCALER   16
#elif(RTC_PRESCALER_gc == RTC_PRESCALER_DIV64_gc)
    #define RTC_PRESCALER   64
#elif(RTC_PRESCALER_gc == RTC_PRESCALER_DIV256_gc)
    #define RTC_PRESCALER   256
#elif(RTC_PRESCALER_gc == RTC_PRESCALER_DIV1024_gc)
    #define RTC_PRESCALER   1024
#else
    #error "Invalid RTC_PRESCALER_gc"
#endif

#define RTC_CNT_FREQ    (RTC_CLK_FREQ/RTC_PRESCALER)

// 1-minute period
#define RTC_PER_VALUE   (RTC_CLK_FREQ*60/RTC_PRESCALER)

#if(RTC_CALENDAR_ENABLE)
//==============================================================================
// Calendar Variables
//==============================================================================
static uint16_t Cal_year;
static uint8_t  Cal_month;
static uint8_t  Cal_day;
static uint8_t  Cal_dayofweek;
static uint8_t  Cal_hour;
static uint8_t  Cal_minute;

static uint8_t DST_observed;
static uint8_t DST_on;

static calendar_alarm_t *Alarm_first;

//==============================================================================
// Calendar Functions
//==============================================================================

/**
* \brief Calculates and fills in the \c day_of_week member based on the date.
* \param T #calendar_time_t object
**/
static void calc_DOW(calendar_time_t *T){
    const uint8_t t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    uint16_t y;
    y = T->year - (T->month < 3);
    T->dayofweek = (y + y/4 - y/100 + y/400 + t[T->month-1] + T->day) % 7;
}

//------------------------------------------------------------------------------
void calendar_set_time(calendar_time_t *T){
    uint8_t intctrl;
    
    if(T->dayofweek == UNKNOWN_DOW){
        calc_DOW(T);
    }
    
    // Stop RTC
    RTC.CTRL = RTC_PRESCALER_OFF_gc;
    intctrl = RTC.INTCTRL;
    RTC.INTCTRL = 0;
    
    // Wait for sync if needed
    while(RTC.STATUS & RTC_SYNCBUSY_bm);
    
    // Update
    RTC.CNT = (uint16_t)T->second * RTC_CNT_FREQ;
    Cal_year = T->year;
    Cal_month = T->month;
    Cal_day = T->day;
    Cal_dayofweek = T->dayofweek;
    Cal_hour = T->hour;
    Cal_minute = T->minute;
    
    // Restart RTC
    RTC.INTCTRL = intctrl;
    RTC.CTRL = RTC_PRESCALER_gc;
}

//------------------------------------------------------------------------------
void calendar_set_DST(uint8_t observed, uint8_t enabled){
    uint8_t intctrl;
    intctrl = RTC.INTCTRL;
    RTC.INTCTRL = 0;
    
    DST_observed = observed;
    DST_on = enabled;
    
    RTC.INTCTRL = intctrl;
}

//------------------------------------------------------------------------------
void calendar_get_time(calendar_time_t *T){
    // Wait for sync if needed
    while(RTC.STATUS & RTC_SYNCBUSY_bm);
    
    // Copy state
    T->second = RTC.CNT / RTC_CNT_FREQ;
    T->year = Cal_year;
    T->month = Cal_month;
    T->day = Cal_day;
    T->dayofweek = Cal_dayofweek;
    T->hour = Cal_hour;
    T->minute = Cal_minute;
}

//------------------------------------------------------------------------------
void calendar_add_alarm(calendar_alarm_t *A){
    uint8_t intctrl;
    intctrl = RTC.INTCTRL;
    RTC.INTCTRL = 0;
    
    // Insert alarm to head of list
    A->next = Alarm_first;
    Alarm_first = A;
    
    RTC.INTCTRL = intctrl;
}

//------------------------------------------------------------------------------
void calendar_remove_alarm(calendar_alarm_t *A){
    uint8_t intctrl;
    intctrl = RTC.INTCTRL;
    RTC.INTCTRL = 0;
    
    if(A && Alarm_first){
        
        // Check edge case: if it is the first one in the list
        if(A == Alarm_first){
            Alarm_first = Alarm_first->next;
        }else{
            calendar_alarm_t *alarm_prev = Alarm_first;
            calendar_alarm_t *alarm = Alarm_first->next;
            
            // Find alarm and remove from list
            while(1){
                if(alarm == A){
                    alarm_prev->next = alarm->next;
                    break;
                }
                
                // goto next
                if(alarm){
                    alarm_prev = alarm;
                    alarm = alarm->next;
                }else{
                    break;
                }
            }
        }
    }
    
    RTC.INTCTRL = intctrl;
}

//------------------------------------------------------------------------------
static uint8_t days_in_current_month(){
    const uint8_t n[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    uint8_t days;
    
    days = n[Cal_month-1];
    if(Cal_month == 2){
        // February. Check if leap year
        if((Cal_year % 4) == 0){
            if((Cal_year % 100) != 0){
                days++;
            }else if((Cal_year % 400) == 0){
                days++;
            }
        }
    }
    return(days);
}
//------------------------------------------------------------------------------
static bool inc_day(void){
    if(Cal_dayofweek == 6){
        Cal_dayofweek = 0;
    }
    
    if(Cal_day == days_in_current_month(Cal_month, Cal_year)){
        // signal to increment month
        return(true);
    }
    
    return(false);
}

//------------------------------------------------------------------------------
static void adjust_for_dst(void){
    // Function is only called if DST is observed, and just rolled over to a new hour
    
    if(DST_on == 0){
        // DST starts on the second Sunday in March,
        //  moving forward from 2:00 a.m. to 3:00 a.m.
        if((Cal_month == 3) // March
            && (Cal_dayofweek == 0) // Sunday
            && (Cal_day >= 8) && (Cal_day <= 14) // Second Sunday
            && (Cal_hour == 2) // 2:00 AM
        ){
            Cal_hour = 3;
            DST_on = 1;
        }
    }else{
        // DST ends on the first Sunday in November,
        //  moving back from 2:00 a.m. to 1:00 a.m
        if((Cal_month == 11) // November
            && (Cal_dayofweek == 0) // Sunday
            && (Cal_day >= 1) && (Cal_day <= 7) // First Sunday
            && (Cal_hour == 2) // 2:00 AM
        ){
            Cal_hour = 1;
            DST_on = 0;
        }
    }
}

//------------------------------------------------------------------------------
static void check_alarms(void){
    calendar_alarm_t *alarm = Alarm_first;
    
    while(alarm){
        // Check if the alarm should fire
        
        if(((0x01 << Cal_dayofweek) & alarm->dayofweek_mask)
            && (Cal_hour == alarm->hour)
            && (Cal_minute == alarm->minute)
        ){
            // Trigger callback
            if(alarm->callback){
                alarm->callback(alarm->callback_data);
            }
        }
        
        alarm = alarm->next;
    }
}

//------------------------------------------------------------------------------
ISR(RTC_OVF_vect){
    // Increment Time
    if(Cal_minute == 59){
        Cal_minute = 0;
        if(Cal_hour == 23){
            Cal_hour = 0;
            if(inc_day()){
                // inc month
                Cal_day = 1;
                if(Cal_month == 12){
                    Cal_month = 1;
                    Cal_year++;
                }else{
                    Cal_month++;
                }
            }
        }else{
            Cal_hour++;
            
            // If USA Daylight savings is observed, check if adjustment must be made
            if(DST_observed){
                adjust_for_dst();
            }
        }
    }else{
        Cal_minute++;
    }
    
    check_alarms();
}

#endif // RTC_CALENDAR_ENABLE

//==============================================================================
// Event Timer Variables
//==============================================================================
#if(RTC_TIMER_ENABLE)

static timer_t *Timer_first;
static uint16_t Timer_tickref;

//==============================================================================
// Event Timer Functions
//==============================================================================

/**
* \brief Updates timer lists based on new_cnt value
* 
* Time elapsed is ambiguous if new_cnt == Timer_tickref.
* To clear this up, assumption will be based on is_isr:
*   is_isr == true: Time elapsed is RTC_PER_VALUE
*   is_isr == false: Time elapsed is 0
* 
* \warning Must call this from within an atomic block!
* 
* \param new_cnt Latest RTC.CNT value
* \param is_isr Set to 1 if calling from COMP ISR.
**/
static void update_tickref(uint16_t new_cnt, uint8_t is_isr){
    
    if(Timer_first){
        uint16_t ticks_elapsed;
        
        // Calculate time elapsed since last update
        if(new_cnt == Timer_tickref){
            if(is_isr){
                ticks_elapsed = RTC_PER_VALUE;
            }else{
                ticks_elapsed = 0;
            }
        }else if(new_cnt > Timer_tickref){
            ticks_elapsed = new_cnt - Timer_tickref;
        }else{
            // wrapped
            ticks_elapsed = RTC_PER_VALUE - Timer_tickref;
            ticks_elapsed += new_cnt;
        }
        
        // Update first timer
        if(ticks_elapsed > Timer_first->ticks_remaining){
            // something went wrong... Zero out the timer
            Timer_first->ticks_remaining = 0;
        }else{
            Timer_first->ticks_remaining -= ticks_elapsed;
        }
    }
    Timer_tickref = new_cnt;
}

//------------------------------------------------------------------------------
/**
* \brief Insert timer into linked list
* \warning Must call this from within an atomic block!
* \param t Timer to insert
**/
static void insert_timer(timer_t *t){
    timer_t *t_prev = NULL;
    timer_t *t_current = Timer_first;
    
    while(1){
        if(t_current == NULL){
            // Got to end of list. Append
            if(t_prev){
                t_prev->next = t;
            }else{
                // List was empty
                Timer_first = t;
            }
            return;
        }
        
        // See if t should go before or after t_current
        if(t->ticks_remaining < t_current->ticks_remaining){
            // Insert before
            t_prev->next = t;
            t->next = t_current;
            
            // update
            t_current->ticks_remaining -= t->ticks_remaining;
            return;
        }else{
            // Seek to next
            t->ticks_remaining -= t_current->ticks_remaining;
            t_prev = t_current;
            t_current = t_current->next;
        }
            
    }
}

//------------------------------------------------------------------------------
/**
* \brief First timer in list may have changed. Update RTC.COMP
* \warning Must call this from within an atomic block!
**/
static void update_COMP(){
    if(Timer_first){
        // Update COMP based on first timer. Assume tickref was just updated
        if(Timer_first->ticks_remaining < RTC_PER_VALUE){
            uint16_t new_comp;
            // Next timer is due to expire soon!
            new_comp = Timer_tickref + Timer_first->ticks_remaining;
            if(new_comp >= RTC_PER_VALUE){
                new_comp -= RTC_PER_VALUE;
            }
            RTC.COMP = new_comp;
        }else{
            // Leave COMP as-is
        }
        
        // Enable COMP Interrupt
        RTC.INTCTRL |= RTC_COMPINTLVL;
    }else{
        // Disable COMP Interrupt
        RTC.INTCTRL &= ~(RTC_COMPINTLVL_gm);
    }
}

//------------------------------------------------------------------------------
ISR(RTC_COMP_vect){
    uint16_t current_CNT;
    timer_t *t = NULL;
    
    // Get CNT value
    current_CNT = RTC.CNT;
    
    // Check if timer expired
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
        update_tickref(current_CNT,1);
        
        if(Timer_first){
            if(Timer_first->ticks_remaining == 0){
                // Timer expired
                
                // Save pointer to timer and remove from list
                t = Timer_first;
                Timer_first = Timer_first->next;
                
                
                // if repeat, reload ticks_remaining and insert_timer()
                if(t->ticks_reload){
                    t->ticks_remaining = t->ticks_reload;
                    insert_timer(t);
                }
                
                update_COMP();
            }
        }
    }
    
    // Call callback
    if(t){
        if(t->callback){
            t->callback(t->callback_data);
        }
    }
    
    
    // Check for any more expired timers. Repeat until all are cleared
    while(t){
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
            if(Timer_first){
                if(Timer_first->ticks_remaining == 0){
                    // Timer expired
                    
                    // Save pointer to timer and remove from list
                    t = Timer_first;
                    Timer_first = Timer_first->next;
                    
                    // if repeat, reload ticks_remaining and insert_timer()
                    if(t->ticks_reload){
                        t->ticks_remaining = t->ticks_reload;
                        insert_timer(t);
                    }
                    
                    update_COMP();
                }
            }
        }
        
        // Call callback
        if(t){
            if(t->callback){
                t->callback(t->callback_data);
            }
        }
    }
    
}

//------------------------------------------------------------------------------
void timer_start(timer_t *timerid, struct timerctl *settings){
    uint16_t current_CNT;
    if(settings){
        // creating new timer. Copy parameters
        timerid->ticks_remaining = settings->interval;
        timerid->callback = settings->callback;
        timerid->callback_data = settings->callback_data;
        if(settings->repeat){
            timerid->ticks_reload = timerid->ticks_remaining;
        } else {
            timerid->ticks_reload = 0;
        }
    }
    
    timerid->next = NULL;
    
    // Get CNT value
    while(RTC.STATUS & RTC_SYNCBUSY_bm);
    current_CNT = RTC.CNT;
    
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
        update_tickref(current_CNT,0);
        insert_timer(timerid);
        if(timerid == Timer_first){
            update_COMP();
        }
    }
}

//------------------------------------------------------------------------------
void timer_stop(timer_t *timerid){
    uint16_t current_CNT;
    uint16_t ticks_sum = 0;
    
    // Get CNT value
    while(RTC.STATUS & RTC_SYNCBUSY_bm);
    current_CNT = RTC.CNT;
    
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
        timer_t *t_prev = NULL;
        timer_t *t_current = Timer_first;
        
        update_tickref(current_CNT,0);
        
        // Remove timerid from list
        //  (Keep track of cumulative ticks_remaining)
        while(t_current){
            if(t_current == timerid){
                // found match
                if(t_prev){
                    t_prev->next = t_current->next;
                }else{
                    // t_current is Timer_first
                    Timer_first = Timer_first->next;
                }
                break;
            } else {
                // goto next
                ticks_sum += t_current->ticks_remaining;
                t_prev = t_current;
                t_current = t_current->next;
            }
        }
        
        update_COMP();
    }
    
    // Update timerid
    timerid->next = NULL;
    timerid->ticks_remaining += ticks_sum;
}
#endif // RTC_TIMER_ENABLE

//==============================================================================
// General RTC Functions
//==============================================================================
void rtc_init(void){
    RTC.CTRL = RTC_PRESCALER_OFF_gc;
    CLK.RTCCTRL = RTC_CLKSRC_gc | CLK_RTCEN_bm;
    
    // Set period to 1 minute
    RTC.PER = RTC_PER_VALUE;
    RTC.CNT = 0x0000;
    
    RTC.INTFLAGS = RTC_COMPIF_bm | RTC_OVFIF_bm;
    RTC.INTCTRL = RTC_COMPINTLVL_OFF_gc | RTC_OVFINTLVL;
    
    #if(RTC_CALENDAR_ENABLE)
        Alarm_first = NULL;
        
        // Set to a valid date
        Cal_year = 2000;
        Cal_month = 1;
        Cal_day = 1;
        Cal_dayofweek = 0;
        Cal_hour = 0;
        Cal_minute = 0;
        DST_observed = 0;
        DST_on = 0;
    #endif
    
    #if(RTC_TIMER_ENABLE)
        Timer_first = NULL;
    #endif
    
    // Start RTC
    RTC.CTRL = RTC_PRESCALER_gc;
}

//------------------------------------------------------------------------------
void rtc_uninit(void){
    RTC.CTRL = RTC_PRESCALER_OFF_gc;
    RTC.INTCTRL = RTC_COMPINTLVL_OFF_gc | RTC_OVFINTLVL_OFF_gc;
    CLK.RTCCTRL = 0x00;
}
