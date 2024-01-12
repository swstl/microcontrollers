#include "mbed.h"

#define WAIT_TIME_MS 100ms

AnalogIn potmeter(PC_0);           // Potensiometer
DigitalIn button(PA_1, PullUp);    // knapp

PwmOut green1(PA_7);               // grønn diode 1
PwmOut green2(PA_2);               // grønn diode 2             /
PwmOut red(PA_15);                 // rød led

int main() {
  while (true) {

    printf("volt: %f\n", (float)potmeter); //bing chilling

    if (button.read() == 1) { // dersom knappen ikke er trykket inn
        if (potmeter < 1.0/3) {
            green1.write((potmeter-0.01)/0.33); // setter % lysstyrken til grønn diode basert på skala fra 0.01(fjerner småspenninger) - 0.33 
            green2 = 0;
            red = 0;
        } 
        
        else if (potmeter > 1.0/3 && potmeter < 2.0/3) {
            green2.write((potmeter-0.33)/0.33); // setter % lyssyrken til den 2. grønn diode basert på input mellom 0.33 - 0.66 
            green1 = 1;
            red = 0;
        } 
        
        else {
            red.write((potmeter-0.66)/0.33); // setter % lyssyrken til rød diode basert på input mellom 0.66 - 0.99  
            green2 = 1;
            green1 = 1;
        }

    } else { // når knappen er trykket
      red = 1;
      green2 = 1;
      green1 = 1;
      printf("LED on!\n");
    }

    ThisThread::sleep_for(WAIT_TIME_MS); // 10x per sekund
  }
} //https://tools.uia.no/bitbucket/projects/IKT104G23V/repos/sanderw/browse/assignment_2/main.cpp?at=refs%2Fheads%2Fmaster
