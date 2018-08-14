#ifndef __I2C_h
#define __I2C_h

class I2C
{
private:
    int dev;

public:
    I2C();
    I2C(unsigned int addr);
    I2C(const char* path, unsigned int addr);

    int open(const char* path, unsigned int addr);
    int setSlaveAddress(unsigned int addr);
    int close();

    int write(unsigned char data);
    int read();
};

#endif