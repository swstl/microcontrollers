/**
 * @file   main.cpp
 * @authors Sander Wesstøl, Linor Ujkani, Erlend Tregde, Luka Vulovic
 */

#include "Callback.h"
#include "DFRobot_RGBLCD.h"
#include "InterruptIn.h"
#include "ThisThread.h"
#include "mbed_rtc_time.h"
#include "data.h"
#include "wifi.h"
#include "WifiThread.h"
#include "mbed.h"
#include "json.hpp"
#include "HTS221Sensor.h"
#include <cstdio>
#include <ctime>
#include <string>
#include "Alarm.h"


#define WAIT_TIME_MS 500ms
#define length_of_screen 16
#define ButtonCooldown 200              // number of ms before a new buttonpress registrers after first press 
#define additional_screens 5            // default skjermen er ikke tatt med i tellingen

#define button1(x) button1(x, PullUp)   // right button (alone)
#define button2(x) button2(x, PullUp)   // right button (left side)     // <-
#define button3(x) button3(x, PullUp)   // middle button (left side)    // <- the 3 buttons
#define button4(x) button4(x, PullUp)   // left button (left side)      // <-

#define SDA1 PB_9                       // lcd communication channels
#define SCL1 PB_8

#define SDA2 PB_11                      // sensor communication channels
#define SCL2 PB_10


// components
DevI2C com(SDA2, SCL2);                 //I2C2
HTS221Sensor sensor(&com);              // HUMIDITY AND TEMPERATURE SENSOR
DFRobot_RGBLCD lcd(length_of_screen, 2, SDA1, SCL1);
InterruptIn button1(D2);
InterruptIn button2(D3);
InterruptIn button3(D4);
InterruptIn button4(D5);
AnalogIn Potmeter(A5);


// program code
Thread Controller(osPriorityNormal, MBED_CONF_RTOS_MAIN_THREAD_STACK_SIZE); // made stacksize larger
static Data data;                                                           // Data class containing most of the data, which is displaied on the lcd 
Alarm alarm;                                                                // Alarm class which operates the alarm
char new_location[length_of_screen] = {32};

int press = 0;                          // Anumber used to check how many times the switch screen button (button 1) is pressed.
int cursor = 0;                         // A cursor value used to orientate the cursor on the display.
bool clear_display = false;             // A variable used to clear the display once after each new screen.

int hour = 0;                           // Int value of the hours to be set as the alarm. Gathered by the potentiometer.
int minute = 0;                         // Int value of the minutes to be set as the alarm. Gathered by the potentiometer. 
bool start_alarm = false;               // Boolean used to toggle the alarm on and off.
bool blink = false;                     // Boolean used to create the blinking effect in the alarm screen.
bool hour_cursor = true;               // Boolean used to select which of the clock values (hours/minutes) the user is working with. 




/** switch_screen
 *  this is the function called by the 1st button (lonely one), that takes care of switching screens.
 *  It also saves the newly added location if anything is changed. 
 *  This is controlled by button 1 
 */
void switch_screen(){

    // saves the new location to the data class, if something have been altered.
    if (press == 4 && new_location[0] != 32) {   // marks the location as changed after changing it in screen 4
        data.time.city.clear();
        data.time.city.append(new_location);
        data.location_change = true;
    }

    if (press >= additional_screens)
        press = 0;

    else
        press ++;

    // resets variables used in certain screens
    memset(new_location, 0, sizeof(new_location));
    new_location[0] = 32;

    cursor = 0;
    clear_display = true;

}



/** move_right, move_left, change_character
 *   move_right is triggered by the right of the 3 buttons and moves the cursor one section over,
 *   move_left is the same as move_right just opposite way, which is triggered by the left of the 3 buttons. 
 *   Once the cursor hits the given "max" position, it will teleport to the given "min" position. 
 *   By default will the "min" position be at the start of the screen. 
 *   if a skip value is given the cursor will jump to the tile next to the "skip" position.
 *   change character is triggered by the middle of the 3 buttons, and changes the displayed character.
 */
void move_right(int max, int min = 0, int skip = 0){
    if (skip != 0 && cursor == skip-2)
        cursor ++;

    if (cursor >= max-1)
        cursor = min;

    else
        cursor ++;
}   
void move_left(int max, int min = 0, int skip = 0){
    if (skip != 0 && cursor == skip)
        cursor--;

    if (cursor <= min)
        cursor = max-1;

    else
        cursor --;
}
void change_character(){

    // just to make the first letter CAPITAL. (not that it matters)
    if (cursor == 0){ // first character
        if (new_location[cursor] < 65 || new_location[cursor] >= 90) // character range: A-Z
            new_location[cursor] = 65;
        else
            new_location[cursor]++;
    }

        // lovercase letters:
    else{
        if (new_location[cursor] < 97 || new_location[cursor] >= 122) // character range: a-z
            new_location[cursor] = 97;
        else
            new_location[cursor]++;
    }

}



// button 3
void middle_button(){

    // Location changing screen (screen 4)
    if (press == 4){
        change_character();
    }

        // Alarm screen (screen 1)
    else if (press == 1) {
        start_alarm = !start_alarm;
    }

    else {
        alarm.mute();
    }

}



// button 4
void left_button(){

    if (press == 4){
        move_left(length_of_screen);
    }

    else if (press == 1 && alarm.IsEnabled()){
        hour_cursor = !hour_cursor;
    }

    else {
        alarm.snooze();
    }

}



// Button 2
void right_button(){

    if (press == 4){
        move_right(length_of_screen);
    }

    else if (press == 1 && alarm.IsEnabled()){
        hour_cursor = !hour_cursor;
    }

    else {
        alarm.snooze();
    }

}



/** clear_screen
 *   clears the lcd display screen once every time the screens are switched.
 *   it doesn't clear for each loop to avoid flickering in display.
 *   it also stops the cursor from blinking on the display.
 */
void clear_screen(){
    lcd.blink_off();
    if (clear_display){
        lcd.clear();
        clear_display = false;
    }
}



/** get_clock_values 
 *  Converts the values from the potentiometer into hours and minutes.
 *  Which one it converts the values to, 
 *  is based off of wheter the user is workng on the hour clock,
 *  or the minute clock. Chosen by the (hour_cursor)
 */
void get_clock_values(){
    if (hour_cursor){
        hour = 24*Potmeter;
    }

    else {
        minute = 60*Potmeter;
    }
}



int main(){
    lcd.init();
    sensor.init(nullptr);
    sensor.enable();
    button1.fall(&switch_screen);
    button2.fall(&right_button);
    button3.fall(&middle_button);
    button4.fall(&left_button);


    // used in scrolling display
    int offset = 0;         // offset character to start from
    int size = 0;           // length of text


    /** The controller is a seperate thread that collects all the data from the network,
     *   through http/https requests. It is set to run every 1s, but update the data every 5 minute.
     *   The time could also be longer to reduce network traffic.
     */
    Controller.start(callback(controller, &data));


    // printer ut alle startverdiene så fort de er klare
    while (!data.init){
        lcd.clear();
        lcd.printf("Connecting");
        ThisThread::sleep_for(300ms);
        lcd.printf(".");
        ThisThread::sleep_for(300ms);
        lcd.printf(".");
        ThisThread::sleep_for(300ms);
        lcd.printf(".");
        ThisThread::sleep_for(300ms);
    }


    //First 10% (beginning)
    lcd.clear();
    lcd.printf("Unix epoch time:");
    lcd.setCursor(0, 1);
    lcd.printf("%u", data.time.START_UNIX_RTC);
    ThisThread::sleep_for(2s);

    lcd.clear();
    lcd.printf("Lat: %s", data.time.latitude.c_str());
    lcd.setCursor(0, 1);
    lcd.printf("Lon: %s", data.time.longitude.c_str());
    ThisThread::sleep_for(2s);

    lcd.clear();
    lcd.printf("City:");
    lcd.setCursor(0, 1);
    lcd.printf("%s", data.time.city.c_str());
    ThisThread::sleep_for(2s);
    lcd.clear();
    //First 10% (end)


    while (true) {

        switch (press) {

                // Alarm screen (set, change, enable, disable, etc...)
            case 1:
                clear_screen();
                lcd.setCursor(0, 0);

                if (start_alarm) {
                    lcd.printf("Alarm: enabled  ");
                    get_clock_values();
                    alarm.set(hour, minute);  
                    alarm.start();
                } else {
                    lcd.printf("Alarm: disabled ");
                    alarm.stop();
                }


                lcd.setCursor(0, 1);

                // (Switches between displaying the time, and displaying only a part of it)
                // adds "blinking" to the display, to indicate which part of the clock the user is working on (hours or minutes)
                if (blink && hour_cursor && alarm.IsEnabled()) {
                    lcd.printf("  ");
                    lcd.printf(&alarm.current_time()[2]);
                } else if (blink && !hour_cursor && alarm.IsEnabled()) {
                    lcd.printf(alarm.current_time());
                    lcd.setCursor(3, 1);
                    lcd.printf("  ");
                } else {
                    lcd.printf(alarm.current_time());
                }
                
                blink = !blink;
                break;


                // Room humidity and temperature screen
            case 2:
                clear_screen();

                float humidity;
                float temperature;

                sensor.get_humidity(&humidity);
                sensor.get_temperature(&temperature);

                lcd.setCursor(0, 0);
                lcd.printf("Temp: %.1f C", temperature);
                lcd.setCursor(0, 1);
                lcd.printf("Humidity: %.0f %%", humidity);
                break;


                // Weather report screen
            case 3:
                clear_screen();
                lcd.setCursor(0, 0);
                lcd.printf("%s          ", data.weather.description.c_str());
                lcd.setCursor(0, 1);
                lcd.printf("%s degrees  ", data.weather.degrees.c_str());
                break;


                // Set new location screen
            case 4:
                clear_screen();
                lcd.setCursor(0, 0);
                lcd.printf("Change location:");
                lcd.setCursor(cursor, 1);
                lcd.blink();

                if(new_location[cursor] < 32) // makes all NULL characters invisible when displayed (if not changed). (changes them to ' ')
                    new_location[cursor] = 32;

                lcd.write(new_location[cursor]);
                lcd.setCursor(cursor, 1); // makes sure the cursor doesn't move before character is confirmed
                break;


                // News feeds screen
            case 5:
                clear_screen();

                size = data.news.whole_newsfeed.size(); // size of newsfeed
                lcd.setCursor(0, 0);
                lcd.printf("BBC:");
                lcd.setCursor(0, 1);
                for (int i = 0; i < length_of_screen; i++){
                    if (i + offset >= size)
                        lcd.write(data.news.whole_newsfeed[i+offset-(size)]);
                    else
                        lcd.write(data.news.whole_newsfeed[i+offset]);
                }

                offset++;
                if (offset == size)
                    offset = 0;

                break;


                // default screen (date and time + alarm)
            default:
                clear_screen();

                time_t seconds = time(NULL); // Current calender time based of the UNIX EPOCH TIME
                char date[length_of_screen]; // name of day, date of day, name of month.
                char time[length_of_screen]; // 24 hour clock 

                strftime(date, length_of_screen, "%a %d %b ", localtime(&seconds));
                strftime(time, length_of_screen, "%H:%M", localtime(&seconds));

                lcd.setCursor(0, 0);
                lcd.printf("%s", date);
                lcd.printf("%s", time);

                if(alarm.IsEnabled()){
                    lcd.setCursor(0, 1);
                    lcd.printf("Alarm:   %s", alarm.current_time());
                }

                break;
        }

        // printf("used memory on thread stack: %i\n", Controller->used_stack());
        ThisThread::sleep_for(300ms);
    }

    return 0;
}

