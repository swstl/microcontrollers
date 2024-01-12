/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */
#include "mbed.h"
#include "DFRobot_RGBLCD.h"
#include <chrono>
#include <cstdio>

#define button1 D8
#define button2 D0
#define button3 D4
#define button4 D11
#define SDA1 PB_9
#define SCL1 PB_8
#define WAIT_TIME_MS 100ms


Timer t;
DigitalOut led1(LED1);
PwmOut buzzer(D3);
InterruptIn knapp1(button1, PullUp);
InterruptIn knapp2(button2, PullUp);
InterruptIn knapp3(button3, PullUp);
InterruptIn knapp4(button4, PullUp);
DFRobot_RGBLCD lcd(16, 2, SDA1, SCL1);
float TIME = 60;
float displaytime;


void pause_play(){
    static bool state = true;

    if (state) {
        t.stop();
    } else {
        t.start();
    }
    state = !state;
}


void reset(){
   TIME = 60;
   t.reset();
   t.stop();
}


void add(){
    //add 5 seconds to timer
    TIME += 5;
    t.start();
}


void sub(){
    //subtract 5 seconds from timer
    if (displaytime > 5) {
        TIME -= 5;

    }else if(displaytime <= 5 && displaytime >= 0){
        TIME -= displaytime;
    }
}


int main(){

    knapp1.fall(&pause_play);
    knapp2.fall(&reset);
    knapp3.fall(&add);
    knapp4.fall(&sub);
    lcd.init();
    t.start();
   

    while (true){    
        displaytime = TIME  - (duration_cast<chrono::milliseconds>(t.elapsed_time()).count()) / 1000.0;

        if (displaytime > 0) {
            
            lcd.clear();
            lcd.printf("%.2fs", displaytime);
            buzzer.write(0);
            ThisThread::sleep_for(WAIT_TIME_MS);

        }else {
            lcd.clear();
            lcd.printf("%.2fs", 0);        
            t.stop();
            buzzer.write(0.5);
            ThisThread::sleep_for(WAIT_TIME_MS);
        }        
    }
}


