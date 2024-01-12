#include "mbed.h"
#include "HTS221Sensor.h"
#include "DFRobot_RGBLCD.h"
#include "millis.h"
#include <cstddef>
#include <cstdio>


#define WAIT_TIME_MS 500ms
#define SDA1 PB_9
#define SCL1 PB_8
#define SDA2 PB_11
#define SCL2 PB_10

DigitalIn knapp(D8, PullUp); // den ENE knappen
DevI2C com(SDA2, SCL2); //I2C2
HTS221Sensor sensor(&com); // HUMIDITY AND TEMPERATURE SENSOR
DFRobot_RGBLCD lcd(16, 2, SDA1, SCL1); //LCD DISPLAY


int main() {
    millisStart(); //time since last reboot/flash/(turned on)
    int time = 0; //system time when the button was previously pressed
    int time2 = 0; //system time when humidity was previously displayed
    int time3 = 0; //system time when temperature was previously displayed
    bool button = true; //switched state for each button press
    float humidity;
    float temperature = 1;
    lcd.init();

    if (sensor.init(NULL) != 0) {
        printf("Failed to init!\n");
    }

    if (sensor.enable() != 0) {
        printf("Failed to enable!\n");
    }

    while (true) {
        sensor.get_humidity(&humidity);
        sensor.get_temperature(&temperature);
        
        
        if (!knapp){ //switches "button" state when pressed. Does also work when held down
            if (millis() - time >= 200) {
                button = !button;
                printf("button is pressed\n");
            }
            time = millis(); //updates times when pressed
        }


        if (button) {
            if (millis() - time2 >= 1000) { // displays humidity every second
                lcd.clear();
                printf("Humidity: %.2f", humidity);
                lcd.printf("Humidity: %.2f", humidity);
                lcd.setRGB(255 - 2.55*humidity, 255 - 2.55*humidity, 255); //changes backround color based on humidity %. (0% = white, 100% = blue)
                time2 = millis(); //updates times when displayed 
            }


        } else {
            if (millis() - time3 >= 1000) { //displays humidity every second
                lcd.clear();
                lcd.printf("Temp: %.2fc", temperature);

                if (temperature < 20) {
                    lcd.setRGB(0, 0, 255);
                } else if (temperature >= 20 && temperature <= 24) {
                    lcd.setRGB(255, 165, 0);
                } else {
                    lcd.setRGB(255, 0, 0);
                }
                time3 = millis(); //updates times when displayed
            }
        }
    }
} //https://tools.uia.no/bitbucket/projects/IKT104G23V/repos/sanderw/browse/assignment_3/main.cpp?at=master