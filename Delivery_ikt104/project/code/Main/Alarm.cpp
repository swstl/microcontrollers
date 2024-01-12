/**
 * @file   Alarm.cpp
 * @authors Sander Wesstøl, Linor Ujkani
 */

#include "EventFlags.h"
#include "Alarm.h"
#include "ThisThread.h"
#include <cstdio>
#include <string>

#define FLAG (1 << 0)
#define seconds_of_ringing 600     // Equivalent to 10 min [{In the video, this is set to 10s}]
#define seconds_of_mute 300     // Equivalent to 5 min [{In the vide, this is set to 5s}]


Alarm::Alarm() :    alarm_time("00:00"),
                    thread(osPriorityNormal) 
{
    // passes it's own control_alarm function to the thread.
    thread.start([this]{control_alarm();});
}


void Alarm::control_alarm(){

    while (true){

        // if the alarm is stopped the buzzer wil be set to 0;
        if (!_state) {
            buzzer.write(0);
        }


        // flag which is raised in main at screen 1
        eventflag.wait_all(FLAG, osWaitForever, false);

        time_t seconds = time(NULL);
        char time[6];  // 24 hour clock 

        strftime(time, sizeof(time), "%H:%M", localtime(&seconds));

        // if the alarm time is the same as the real time the buzzer will start.
        if (std::strcmp(alarm_time.c_str(), time) == 0 && !_alarm){
            buzzer.write(0.5);
            timer_before_stopping.start();
            _alarm = true;
        } 


        // if the buzzer has been on for 10 min in a row, it will stop.
        if (timer_before_stopping.read() >= seconds_of_ringing){
            buzzer.write(0);
            timer_before_stopping.reset();
            timer_before_stopping.stop();
            _alarm = false;
        }


        // if the snooze button is hit, the buzzer will be muted and the 10 min timer reset.         
        if (_snooze && _alarm) {
            buzzer.write(0);
            timer_before_stopping.reset();
            timer_before_stopping.stop();
            timer_after_snooze.start();
            _snooze = false;
        }


        // after the buzzor has been muted for 5 min, it will start again (for another 10 min).
        if (timer_after_snooze.read() >= seconds_of_mute) {
            buzzer.write(0.5);
            timer_before_stopping.start();
            timer_after_snooze.reset();
            timer_after_snooze.stop();
        }


        // if the alarm isn't on, or muted, no timers should be counting. Also the buzzer will be quiet.
        if (!_alarm || _mute){
            buzzer.write(0);
            timer_after_snooze.reset();
            timer_after_snooze.stop();
            timer_before_stopping.reset();
            timer_before_stopping.stop();
            _alarm = false;
        }

        ThisThread::sleep_for(200ms);
    }
}


void Alarm::start(){
    _state = true;
    eventflag.set(FLAG);    
}


void Alarm::stop(){
    _state = false;
    eventflag.clear(FLAG);
}


void Alarm::snooze(){
    _snooze = true;
}

void Alarm::mute(){
    _mute = true;
}


bool Alarm::IsEnabled(){
    return _state;
}


void Alarm::set(const unsigned int hour, const unsigned int minute){
    if (hour <= 9 ){
        alarm_time[0] = '0';
        alarm_time[1] = std::to_string(hour)[0];
    }

    else {
        alarm_time[0] = std::to_string(hour)[0];
        alarm_time[1] = std::to_string(hour)[1];
    }


    if (minute <= 9){
        alarm_time[3] = '0';
        alarm_time[4] = std::to_string(minute)[0];
    }

    else {
        alarm_time[3] = std::to_string(minute)[0];
        alarm_time[4] = std::to_string(minute)[1];

    }

}

const char* Alarm::current_time(){
    return alarm_time.c_str();
}
