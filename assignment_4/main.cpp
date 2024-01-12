#include "DFRobot_RGBLCD.h"
#include "mbed.h"
#include "Timer.h"
#include <chrono>
#include <ctime>

#define WAIT_TIME_MS 100ms
#define button1 D8
#define button2 D0
#define SDA1 PB_9
#define SCL1 PB_8
const uint32_t TIME_MS = 10000;
volatile int b1 = 1;

DFRobot_RGBLCD lcd(16, 2, SDA1, SCL1);
InterruptIn knapp1(button1, PullUp);
InterruptIn knapp2(button2, PullUp);
Watchdog &dogshit = Watchdog::get_instance();
Timer _clock;

void interrupt1(void){
    b1 = !b1;
}

void interrupt2(void){
    dogshit.kick();
}

int main() {

  knapp1.fall(&interrupt1);
  knapp2.fall(&interrupt2);


  dogshit.start(TIME_MS);
  lcd.init();
  _clock.reset();

  while (true) {
    if (b1) {

      _clock.start();
      lcd.clear();
      lcd.printf(
          "%.2fs",
          (duration_cast<chrono::milliseconds>(_clock.elapsed_time()).count()) /
              1000.0);
      ThisThread::sleep_for(WAIT_TIME_MS);

    }else {
      _clock.stop();
      dogshit.start(TIME_MS);
    }
  }
}
