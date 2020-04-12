#include "I2C.h"
#include <sys/ioctl.h>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <stdlib.h>
#include <cerrno>
#include <string.h>
#include <errno.h>

using namespace std;

I2C::I2C()
{

}

I2C::I2C(unsigned int addr)
{
    I2C::open("/dev/i2c-1", addr);
}

I2C::I2C(const char* path, unsigned int addr)
{
    I2C::open(path, addr);
} 

int I2C::open(const char* path, unsigned int addr)
{
    dev = ::open(path, O_RDWR);

    if(dev < 0){
        cerr << "Could not load I2C Device " << path << strerror(errno) << endl; 
        return -1;
    }else {
        return setSlaveAddress(addr);
    }
}

int I2C::setSlaveAddress(unsigned int addr)
{
    if(ioctl(dev, I2C_SLAVE, addr) < 0){
        cerr << "Could not bind I2C Device to address 0x" 
            << hex << addr << dec << endl;
        return -1;
    }else {
        cout << "I2C Device to address 0x" << hex << addr 
            << dec << "is open." << endl;
    }
    return 0;
}

int I2C::close()
{
    if(::close(dev) < 0){
        cerr << "Error closing I2C File. " << endl;
        return -1;
    }else {
        cout << "Close I2C File correctly" << endl;
    }
    return 0;
}

int I2C::write(unsigned char data) throw(int)
{
    if(::write(dev, &data, 1) != 1){
        //cerr << "Error while writing to I2CDevice" << endl;
        //throw string("Error while writing to I2CDevice");
        printf("Error while writing to I2CDevice. Errno: %i\n", errno);
        printf("Error Message: %s\n", strerror(errno));
        return -1;
        //return -1;
    }
    return data;
}

int I2C::read() throw(int)
{
    char data = '\0';
    if(::read(dev, &data, 1) != 1){
        //cerr << "Cannot read from I2CDevice" << endl;
        return -1;
        //throw string("Error while reading from I2CDevice");
        //return -1;
    }
    return data;
}

