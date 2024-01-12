#include "mbed.h"
#include <functional>

//threads
typedef struct{
    Thread td1;
    Thread td2;
    Thread td3;
}threads;

//data 
typedef struct {
    int minute_counter;
    int second_counter;
    Mutex mutex;
}Counter;

//thread 2
void add_sec(Counter *clock){
    while (true) {

        clock->mutex.lock();
        if (clock->second_counter == 60)
            clock->second_counter = 0;
        clock->second_counter += 1;
        clock->mutex.unlock();

        ThisThread::sleep_for(1s);

    }
}

//thread 1
void add_min(Counter *clock){
        while (true) {
        
        clock->mutex.lock();
        clock->minute_counter += 1;
        clock->mutex.unlock();

        ThisThread::sleep_for(60s);

    }
}

//thread 3
void print_value(Counter *clock){
    while (true) {

        clock->mutex.lock();
        std::printf("Current time: %i minutes, and %i seconds\n", clock->minute_counter, clock->second_counter);
        clock->mutex.unlock();

        ThisThread::sleep_for(500ms);
    }
}


//main thread (could also be used instead of thread 3)
int main(){
    Counter clock;
    threads thread;
    thread.td1.start(callback(add_min, &clock));
    thread.td2.start(callback(add_sec, &clock));
    thread.td3.start(callback(print_value, &clock));
    thread.td1.set_priority(osPriorityAboveNormal);
    thread.td2.set_priority(osPriorityAboveNormal);

    while (true) {
        std::printf("Now running 3 threads\n"); 
        thread.td1.join();
        thread.td2.join();
        thread.td3.join();
        ThisThread::sleep_for(500ms);
    }
}
