//  SOURCE: https://www.kernel.org/doc/Documentation/i2c/dev-interface */
//  Useful package:
//  sudo apt install i2c-tools for i2c scanning (i2cdetect -y 1)
//  To compile:
//  (run these g++ commands from the Examples directory)
/*
mkdir build
mkdir dist
g++ -c  -I../../Include -o build/i2cMux.o ../../Source/i2cMux.cpp
g++ -c -I../../Include -o build/i2cTestMux.o i2cTestMux.cpp
g++ -I../../Include -o dist/i2cTestMux build/i2cMux.o build/i2cTestMux.o -li2c
*/
//  To run:
/*
./dist/i2cTestMux
*/

//  i2c test that initializes the Sparkfun I2C Mux, then sets and gets the port
//  using a modified Sparkfun I2C Mux library
#include "i2cMux.h"
#include <stdio.h>          //printf
#include <stdlib.h>         //exit()


int main()
{
    QWIICMUX mux;   //can declare additional muxes (ex: mux1, mux2, etc) here if using multiple muxes
    int i2cBus = 1; //i2c bus 1 (can also use 0)
    uint8_t i2cDeviceAddress = 0x70;

    if (!mux.begin(i2cBus, i2cDeviceAddress))
    {
        printf("Mux not detected. Freezing...\n");
        // while (1);
        exit(0);
    }
    printf("Mux detected\n");
    mux.setPort(0); //0-7
    uint8_t currentPortNumber = mux.getPort();
    printf("CurrentPort: %d \n", currentPortNumber);

}






