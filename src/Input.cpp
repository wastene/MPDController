#include "Input.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>

using namespace std;

Input::Input(unsigned char pin)
{
    this->pin = pin;

    // Export Pin
    int gpioExport = ::open("/sys/class/gpio/export", O_RDWR);

    const string pinString = to_string(int(pin));
    if(gpioExport >= 0)
    {
        ::write(gpioExport, pinString.c_str(), pinString.length());

        ::close(gpioExport);
    }

    // Open Pin as Input
    int direction = ::open((string("/sys/class/gpip/gpio")+pinString+string("/direction")).c_str(), O_RDWR);

    if(direction >= 0)
    {
        ::write(direction, "in", 2);

        ::close(direction);
    }
}

Input::~Input()
{
    int gpioUnexport = ::open("/sys/class/gpio/unexport", O_RDWR);

    if(gpioUnexport >= 0)
    {
        const string pinString = to_string(int(pin));
        ::write(gpioUnexport, pinString.c_str(), pinString.length());

        ::close(gpioUnexport);
    }
}

int Input::read()
{
    const string pinString = to_string(int(pin));
    int gpioValFD = ::open((string("/sys/class/gpio/gpio")+pinString+string("/value")).c_str(), O_RDWR);

    if(gpioValFD >= 0)
    {
        char value;
        ::read(gpioValFD, &value, 1);

        ::close(gpioValFD);

        return stoi(string(&value));
    }
}