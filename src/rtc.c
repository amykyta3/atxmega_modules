
#include <stdint.h>
#include <stdbool.h>

#include <avr/io.h>

#include "rtc.h"

//------------------------------------------------------------------------------

#define RTC_CLK_FREQ    1024

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


#if(RTC_CALENDAR_ENABLE)
    #define RTC_OVFINTLVL RTC_OVFINTLVL_LO_gc
#else
    #define RTC_OVFINTLVL RTC_OVFINTLVL_OFF_gc
#endif

#if(RTC_TIMER_ENABLE)
    #define RTC_COMPINTLVL RTC_COMPINTLVL_MED_gc
#else
    #define RTC_COMPINTLVL RTC_COMPINTLVL_OFF_gc
#endif


//------------------------------------------------------------------------------
#if(RTC_CALENDAR_ENABLE)
    static uint16_t Cal_year;
    static uint8_t  Cal_month;
    static uint8_t  Cal_day;
    static uint8_t  Cal_dayofweek;
    static uint8_t  Cal_hour;
    static uint8_t  Cal_minute;

    static uint8_t DST_observed;
    static uint8_t DST_on;
    
    static calendar_alarm_t *Alarm_first
#endif

//------------------------------------------------------------------------------
void rtc_init(void){
    RTC.CTRL = RTC_PRESCALER_OFF_gc;
    CLK.RTCCTRL = CLK_RTCSRC_TOSC_gc | CLK_RTCEN_bm;
    
    // Set period to 1 minute
    RTC.PER = (RTC_CLK_FREQ*60/RTC_PRESCALER);
    RTC.CNT = 0x0000;
    
    RTC.INTFLAGS = RTC_COMPIF_bm | RTC_OVFIF_bm;
    RTC.INTCTRL = RTC_COMPINTLVL_OFF_gc | RTC_OVFINTLVL;
    
    #if(RTC_CALENDAR_ENABLE)
        Alarm_first = Null;
        
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
    
    // Start RTC
    RTC.CTRL = RTC_PRESCALER_gc;
}

//------------------------------------------------------------------------------
void rtc_uninit(void){
    RTC.CTRL = RTC_PRESCALER_OFF_gc;
    RTC.INTTRL = RTC_COMPINTLVL_OFF_gc | RTC_OVFINTLVL_OFF_gc;
    CLK.RTCCTRL = 0x00;
}

//==============================================================================
// Calendar Functions
//==============================================================================
#if(RTC_CALENDAR_ENABLE)

/**
* \brief Calculates and fills in the \c day_of_week member based on the date.
* \param T #calendar_time_t object
* \test Test this...
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
		CalcDOW(T);
	}
    
    // Stop RTC
    RTC.CTRL = RTC_PRESCALER_OFF_gc;
    intctrl = RTC.INTCTRL;
    RTC.INTCTRL = 0;
    
    // Wait for sync if needed
    while(RTC.STATUS & RTC_SYNCBUSY_bm);
    
    // Update
    RTC.CNT = T->second * RTC_CNT_FREQ;
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
    T->second = RTC.CNT * RTC_CNT_FREQ;
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
    if(A && Alarm_first){
        uint8_t intctrl;
        intctrl = RTC.INTCTRL;
        RTC.INTCTRL = 0;
        
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
        
        RTC.INTCTRL = intctrl;
    }
}

//------------------------------------------------------------------------------
static uint8_t days_in_current_month(){
    const uint8_t n[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    uint8_t days;
    
    days = n[Cal_month-1];
    if(Cal_month == 2){
        // February. Check if leap year
        if((Cal_year % 4) == 0){
            days++;
        }else if((Cal_year % 100) == 0){
            days++;
        }else if((Cal_year % 400) == 0){
            days++;
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
