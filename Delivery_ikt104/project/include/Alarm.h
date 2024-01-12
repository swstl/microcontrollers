/**
 * @file   Alarm.h
 * @authors Sander Wesstøl, Linor Ujkani
 */

#ifndef __Alarm__
#define __Alarm__

#include "EventFlags.h"
#include "EventQueue.h"
#include "mbed.h"
#include <string>

static PwmOut buzzer(D6);

class Alarm{
public:

    // Default constructor starts a thread executing the function "control_alarm".
    Alarm();   

    // function that controlls all functions of the alarm (start, stop, mute, etc...).
    void control_alarm();
    // Sets the alarm time to the given hour, and minute.
    void set(const unsigned int hour, const unsigned int minute);

    // Raises a flag, which tells the "control_alarm" thread to start working
    void start();
    // Lowers the flar (clears it), so the thread stops.
    void stop();
    // indicates that alarm is to be snoozed.   
    void snooze(); 
    // indicates that alarm is to be muted.
    void mute();

    // returns true if an alarm is active, and false if not.
    bool IsEnabled();

    // returns the current set alarm time.
    const char* current_time();


private:

    // timers to know when to automatically mute or unsnooze buzzer
    Timer timer_after_snooze;
    Timer timer_before_stopping;

    // thread that keeps track of alarm compared to current time:
    EventFlags eventflag;
    Thread thread;
    
    // value indicating if alarm is enabled or disabled:
    bool _state = false;

    // value indicating if the alarm has gone off
    bool _alarm = false;

    // value indicating if the alarm is snoozed or not
    bool _snooze = false;

    // value indicating if the alarm is muted or not
    bool _mute = false;

    // The time the alarm goes off.
    std::string alarm_time;
};

#endif //__Alarm__