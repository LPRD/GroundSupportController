//  SOURCE: https://www.kernel.org/doc/Documentation/i2c/dev-interface */
//  Useful package:
//  sudo apt install i2c-tools for i2c scanning (i2cdetect -y 1)
//  enable the second i2c port on a raspberry pi by adding "dtparam=i2c_vc=on" to /boot/config.txt
//  To compile:
//  (run these g++ commands from the Examples directory)
/*
mkdir build
mkdir dist
g++ -c  -I../../Include -o build/i2cMux.o ../../Source/i2cMux.cpp
g++ -c  -I../../Include -o build/i2cScaleNAU7802.o ../../Source/i2cScaleNAU7802.cpp
g++ -c -I../../Include -o build/i2cTestScale.o i2cTestScale.cpp
g++ -I../../Include -o dist/i2cTestScale build/i2cMux.o build/i2cScaleNAU7802.o build/i2cTestScale.o -li2c
*/
//  To run:
/*
./dist/i2cTestScale
*/

//  i2c test that gets and sets the port of the Sparkfun I2C Mux,
//  then initializes a scale sensor
//  using modified Sparkfun I2C Mux and NAU7802 Scale libraries 
#include "i2cMux.h"
#include "i2cScaleNAU7802.h"
#include <stdio.h>          //printf
#include <stdlib.h>         //exit()
#include <chrono>           //time
#include <thread>           //std::this_thread::sleep_for
#include <math.h>


#define NumCells 8  //can use more than 8 load cells if using multiple i2c muxes


float m[NumCells]= {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
uint8_t loadCellAvailable[NumCells]= {0, 0, 0, 0, 0, 0, 0, 0};
uint8_t connectedCells= 0;
NAU7802 loadCell[NumCells];
QWIICMUX mux;   //can declare additional muxes (ex: mux1, mux2, etc) here if using multiple muxes


int main()
{
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

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    for(int i=0; i<NumCells; i++){
        //if using multiple muxes with the same sensor on the same port, use disablePort on the mux(es) you are currently not reading from
        //Alternatively, use the setPort method with an out of range port number to disable all ports for that mux
        mux.setPort(i);   //additional logic required if using multiple muxes

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (!loadCell[i].begin(i2cBus, 0x2A, true)){  //0x2A (default Qwiic Scale)
            printf("load cell %d not detected. Please check wiring. \n", i);
        }
        else{
            loadCellAvailable[i] = 1;
            connectedCells++;
            printf("load cell %d detected!\n", i);
        }
    }
    if(connectedCells < 1){
        printf("No load cells detected! Please check wiring. Freezing! \n");
        // while (1);
        exit(0);
    }


    //TODO: load calibration data from a file

    //Load Cell Index-Type/Number      0-S1     1-S2     2-S3     3-S4     4-B1     5-B2     6-B3     7-B4                
    int32_t LC_ZeroOffset[8] =        {30637,   20668,   22068,   22068,   22068,   22068,   22068,   22068}; 
    float LC_CalibrationFactor[8] =   {14200.0, 13700.0, 19863.5, 19863.5, 19863.5, 19863.5, 19863.5, 19863.5};

    // ***** SET OR GET LOAD CELL ZERO OFFSET **********
    for(int i=0; i<NumCells; i++){
        if(loadCellAvailable[i]){   //also a dedicated isConnected method in the NAU7802 library
        mux.setPort(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        int zo = loadCell[i].getZeroOffset();
        printf("current zero offset for load cell %d: %d\n", i, zo);

        // loadCell[i].calculateZeroOffset(64);
        //OR
        loadCell[i].setZeroOffset(LC_ZeroOffset[i]);  //or fetch value from memory

        LC_ZeroOffset[i]= loadCell[i].getZeroOffset();
        printf("new zero offset for load cell %d: %ld\n", i, LC_ZeroOffset[i]);
        }
    }
    printf("\n");
    // *************************************************

    // ***** SET OR GET LOAD CELL CALIBRATION FACTOR **********
    for(int i=0; i<NumCells; i++){
        if(loadCellAvailable[i]){
        mux.setPort(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        float cf = loadCell[i].getCalibrationFactor();
        printf("current calibration factor for load cell %d: %f\n", i, cf);

        // std::this_thread::sleep_for(std::chrono::milliseconds(5000)); //7s delay
        // float calibrationWeight = 5.000;  //kg or whatever units
        // printf("about to set new calibration factor, add %f kg \n", calibrationWeight);
        // std::this_thread::sleep_for(std::chrono::milliseconds(5000)); //2s delay
        // loadCell[i].calculateCalibrationFactor(calibrationWeight, 64);
        //OR
        loadCell[i].setCalibrationFactor(LC_CalibrationFactor[i]); //or fetch value from memory

        float cfNew= loadCell[i].getCalibrationFactor();
        printf("new calibration factor for load cell %d: %f\n\n", i, cfNew);
        }
    }
    printf("\n");
    // *************************************************


    while(1)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        for(int i=0; i<NumCells; i++){
            if(loadCellAvailable[i]){
                mux.setPort(i);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                if(loadCell[i].available()){
                    // long currentReading = loadCell[i].getReading();
                    m[i]= loadCell[i].getWeight(true, 1);  
                    //  up to 113 ms for 10 averaged readings
                    //  up tp 13 ms for 2 averaged readings
                    //  up to .5 ms for 1 reading
                    printf("cell %d: %d g,\n", i, 10*(static_cast<int> (m[i]*100)));
                    // printf("cell %d: %d g,\n", i, round(m[i]*100));
                }
                else{
                    loadCellAvailable[i]= 0;
                    connectedCells--;
                    printf("Issue connecting to load cell %d \n", i);
                }
            }
        }
        printf("\n\n\n");
    }








}






