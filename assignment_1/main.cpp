#include "mbed.h"

#define WAIT_TIME_MS 500 

DigitalOut red(PA_0);
BufferedSerial port(CONSOLE_TX, CONSOLE_RX, 115200);

int main() {

    char input;
    
    while (true) {

        port.read(&input, sizeof(input));

        if ( input == '0') {
            red = 0;
            printf("LED off!\n");

        } else if (input == '1') {
            red = 1;
            printf("LED on!\n");
        }
    
    } 
}

// https://tools.uia.no/bitbucket/projects/IKT104G23V/repos/sanderw/browse/assignment_1/main.cpp?at=refs%2Fheads%2Fmaster