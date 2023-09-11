//  SOURCE: https://www.kernel.org/doc/Documentation/i2c/dev-interface */
//  Useful package:
//  sudo apt install i2c-tools for i2c scanning (i2cdetect -y 1)
//  To compile: (from the i2cTestBasic folder)
/*
mkdir dist
g++ i2cTestBasic.cpp -li2c -o dist/i2cTestBasic
*/
//  To run:
/*
./dist/i2cTestBasic
*/

//  Basic i2c test that gets and sets the port of the Sparkfun I2C Mux
#include <linux/i2c-dev.h>  //can also include in extern block
extern "C" {
    #include <i2c/smbus.h>  //sudo apt install libi2c-dev (add "-l i2c" to compilation)
}
#include <stdio.h>          //snprintf
#include <stdlib.h>         //exit()
#include <fcntl.h>          //For O_RDWR
#include <unistd.h>         //For open(), creat()
#include <sys/ioctl.h>      //ioctl()

int main()
{
    printf("Hello There!\n");
    int file;
    int adapter_nr = 1; //i2c bus 1 (can also use 0)
    char filename[20];

    snprintf(filename, 19, "/dev/i2c-%d", adapter_nr);
    file = open(filename, O_RDWR);
    if (file < 0) {
    exit(1);
    }

    int addr = 0x70;
    //specify i2c address to communicate with
    if (ioctl(file, I2C_SLAVE, addr) < 0) {
    exit(1);
    }

    __u8 reg = 0x10;
    __s32 res;
    char buf[10];
    char buf2[10];

    /*
    *   Using SMBus commands (preferred)
    *   If you need to read data from a specific register on a device:
    *   i2c_smbus_read_word_data(file, reg)
    *   There are also functions to read/write block data
    */
    res = i2c_smbus_read_byte(file);  
    if (res < 0) {
    /* ERROR HANDLING: i2c transaction failed */
        printf("SMBus read error!\n");
    } else {
    /* res contains the read word/byte */
        printf("res is %d\n", res);
    }

    /*
    *   Using I2C Write, equivalent of
    *   i2c_smbus_write_word_data(file, reg, 0x6543)
    */
    buf[0] = 0x10;          //reg;
    // buf[1] = 0x43;       //0x43;
    // buf[2] = 0x65;       //0x65;
    if (write(file, buf, 1) != 1) {
    /* ERROR HANDLING: i2c transaction failed */
        printf("I2C write error!\n");
    } else {
        printf("buf is %d\n", buf[0]);
    }

    /*
    *   Using I2C Read, equivalent of i2c_smbus_read_byte(file)
    *
    */
    if (read(file, buf2, 1) != 1) {
        /* ERROR HANDLING: i2c transaction failed */
        printf("I2C read error!\n");
    } else {
        /* buf[0] contains the read byte */
        printf("buf2 is %d\n", buf2[0]);
    }

}






