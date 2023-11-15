//  SOURCE: https://www.kernel.org/doc/Documentation/i2c/dev-interface */
//  Useful package:
//  sudo apt install i2c-tools for i2c scanning (i2cdetect -y 1)
//  enable the second i2c port on a raspberry pi by adding "dtparam=i2c_vc=on" to /boot/config.txt
//  To compile:
//  (run commands from the tcpScaleTest directory)
/*
make
*/
//  To run:
/*
make run
*/
//  read Makefile to see exact commands

//  i2c test that gets and sets the port of the Sparkfun I2C Mux,
//  then initializes a scale sensor
//  using modified Sparkfun I2C Mux and NAU7802 Scale libraries 
#include "i2cMux.h"
#include "i2cScaleNAU7802.h"
#include <stdlib.h>         //exit()
#include <chrono>           //time
#include <thread>           //std::this_thread::sleep_for
#include <math.h>
#include <arpa/inet.h>
#include <stdio.h>          //printf
#include <string.h>         //strcpy()
#include <string>           //std::string
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream> 

// #include <stdio.h>          //snprintf, fprintf, fopen, etc
// #include <chrono>           //time
// #include <thread>           //std::this_thread::sleep_for
//To debug, add the following lines in the desired location
// FILE *fp;
// fp = fopen("log.txt","a");
// auto begin = std::chrono::steady_clock::now();
// library task
// auto end = std::chrono::steady_clock::now();
// auto diff = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
// fprintf(fp, "it took %d us to collect %u readings!\n", diff.count(), avg);
// fclose(fp);

//can use more than 8 load cells if using multiple i2c muxes
#define NumCells 4
#define CalculateZeroOffset 0
#define CalculateCalibrationFactor 0
#define RunSocket 1
#define PORT 12819   //8080

float m[NumCells] = {0.0f};
int mg[NumCells] = {0}; 
uint8_t loadCellAvailable[NumCells]= {0};
uint8_t connectedCells= 0;
NAU7802 loadCell[NumCells];
QWIICMUX mux;   //can declare additional muxes (ex: mux1, mux2, etc) here if using multiple muxes

class DataPacket {
  public:
    char cmd[16];           //command variable, 4 byte alligned
    // char cmd2[30];          //additional text
    int data[NumCells];   //sensor data
    // float data[NumCells];   //sensor data float
    // std::string text;       //NEED TO BE 4n byte aligned, strings might be a bad idea
};


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

    int32_t LC_ZeroOffset[8] =        {0}; 
    float LC_CalibrationFactor[8] =   {0};

    std::fstream calFile;
    calFile.open("cal.txt", std::ios::in | std::fstream::out);  
    //need fstream::out and std::fstream::trunc if the file doesn't already exist
    if (calFile.is_open()) {
        printf("successfully opened the cal file!\n");
        std::string inputString;
        while(getline(calFile, inputString)){
            std::cout << "inputString is: " << inputString << std::endl;
            std::vector<std::string> calValues;
            std::stringstream s_stream(inputString);
            while(s_stream.good()){
                std::string value;
                getline(s_stream, value, ',');
                calValues.push_back(value);
            }
            if(calValues.at(0) == "Zero Offsets:"){
                std::cout << "Loading Zero Offsets!\n";
                for (int i = 0; i < NumCells; i++) {
                    LC_ZeroOffset[i] = stoi(calValues.at(i+1));
                    // std::cout << "LC_ZeroOffset[" << i << "] is " << LC_ZeroOffset[i] << std::endl;
                }
            }
            if(calValues.at(0) == "Calibration Factors:"){
                std::cout << "Loading Calibration Factors!\n";
                for (int i = 0; i < NumCells; i++) {
                    LC_CalibrationFactor[i] = stof(calValues.at(i+1));
                    // std::cout << "LC_CalibrationFactor[" << i << "] is " << LC_CalibrationFactor[i] << std::endl;
                }
            }

        }
        calFile.close();
    }
    else {
        std::cout << "Error opening calFile!\n";
        std::cout << "Try creating a file 'cal.txt' with comma-separated zero-offset values on the first line:" << std::endl;
        std::cout << "Zero Offsets:,31133,25082,36751,-8367,22068,22068,22068,22068" << std::endl;
        std::cout << "and comma-separated calibration factor values on the second line:" << std::endl;
        std::cout << "Calibration Factors:,14499.2,14598.0,14866.2,14489.6,19863.5,19863.5,19863.5,19863.5" << std::endl;
        std::cout << "(spaces between values after the first comma in each line are ok)\n";
        exit(0);
    }

    // ***** SET OR GET LOAD CELL ZERO OFFSET **********
    for(int i=0; i<NumCells; i++){
        if(loadCellAvailable[i]){   //also a dedicated isConnected method in the NAU7802 library
        mux.setPort(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        int zo = loadCell[i].getZeroOffset();
        printf("current zero offset for load cell %d: %d\n", i, zo);

        if(CalculateZeroOffset){
            loadCell[i].calculateZeroOffset(64);
        }
        else{
            loadCell[i].setZeroOffset(LC_ZeroOffset[i]);  //or fetch value from memory
        }

        LC_ZeroOffset[i]= loadCell[i].getZeroOffset();
        printf("new zero offset for load cell %d: %d\n", i, LC_ZeroOffset[i]);
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

        if(CalculateCalibrationFactor){
            std::this_thread::sleep_for(std::chrono::milliseconds(5000)); //7s delay
            float calibrationWeight = 5.000;  //kg or whatever units
            printf("about to set new calibration factor, add %f kg \n", calibrationWeight);
            std::this_thread::sleep_for(std::chrono::milliseconds(5000)); //2s delay
            loadCell[i].calculateCalibrationFactor(calibrationWeight, 64);
        }
        else{
            loadCell[i].setCalibrationFactor(LC_CalibrationFactor[i]); //or fetch value from memory
        }

        float cfNew= loadCell[i].getCalibrationFactor();
        printf("new calibration factor for load cell %d: %f\n\n", i, cfNew);
        }
    }
    printf("\n");
    // *************************************************

    int status, valread, client_fd;
    struct sockaddr_in serv_addr;
    char hello[] = "new message"; //Hello from client
    //"connection" doesn't work
    
    char buffer[1024] = { 0 };
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
  
    // Convert IPv4 and IPv6 addresses from text to binary form
    // TODO use getenv or main arguments to have a customizable target server address
    // or add the server address to a editable config file that is read in on startup 
    if (inet_pton(AF_INET, "192.168.0.193", &serv_addr.sin_addr)    //127.0.0.1     //192.168.0.113
        <= 0) {
        printf(
            "\nInvalid address/ Address not supported \n");
        return -1;
    }
  
    if(!CalculateZeroOffset && RunSocket){
        if ((status
            = connect(client_fd, (struct sockaddr*)&serv_addr,
                    sizeof(serv_addr)))
            < 0) {
            printf("\nConnection Failed \n");
            return -1;
        }
    }



    DataPacket pkt;
    char zero[16] = "bbbbbbbbbbbbbbb";
    strcpy(pkt.cmd, zero);
    for (int i = 0; i < NumCells; i++) {
        pkt.data[i] = 0;
    }

    while(1)
    {
        int bytesToSend = NumCells*4;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        for(int i=0; i<NumCells; i++){
            if(loadCellAvailable[i]){
                mux.setPort(i);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                if(loadCell[i].available()){
                    // long currentReading = loadCell[i].getReading();
                    m[i]= loadCell[i].getWeight(true, 1);  
                    //  up to 790 ms for 64 averaged readings
                    //  up to 113 ms for 10 averaged readings
                    //  up tp 13 ms for 2 averaged readings
                    //  up to .5 ms for 1 reading
                    int roundFactorGrams = 50;  //round sensor readings to this number of grams
                    int remainder = 1000 / roundFactorGrams;
                    mg[i] = roundFactorGrams*(static_cast<int> (m[i]*remainder));
                    // mg[i] = 10*(static_cast<int> (m[i]*100));
                    pkt.data[i] = mg[i];
                    // pkt.data[i] = m[i];

                    printf("cell %d: %d g \t", i, mg[i]);
                    // printf("cell %d: %d g,\n", i, round(m[i]*100));
                }
                else{
                    loadCellAvailable[i]= 0;
                    connectedCells--;
                    printf("Issue connecting to load cell %d \n", i);
                }
            }
        }
        printf("\n");

        //send data

        //use string.h functions for char arrays
        strcpy(pkt.cmd, "data:int32:");

        // bytesToSend += strlen(pkt.cmd); //does not work, must use whole buffer length
        bytesToSend += 16;  //need to send an amount of bytes that is 4byte aligned
        //NEED TO BE 4 BYTE ALIGNED

        if(!CalculateZeroOffset && RunSocket){
            // printf("sent %d bytes!\n", bytesToSend);
            send(client_fd, &pkt, bytesToSend, 0);

            // valread = read(client_fd, buffer, 1024);    //Move this to a separate task
            // printf("%s\n", buffer);
            // need to add a way to escape from read after a certain amount of time

            int iBuf[1024/4] = { 0 };
            valread = read(client_fd, iBuf, 1024);   
            for (int i = 0; i < NumCells; i++){
                printf("%d\t", iBuf[i]);
            }
            printf("\n");
        
            // float fBuf[1024/4] = { 0 };
            // valread = read(client_fd, fBuf, 1024);
            // for (int i = 0; i < NumCells; i++){
            //     printf("%f\t", fBuf[i]);
            // }
            // printf("\n");
        }


    }


}




