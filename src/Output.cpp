#include "Output.h"

Output::Output(unsigned char pin)
{
    this->pin = pin;

    // TODO: Open Pin
}

Output::~Output()
{
    // TODO: Close pin
}

int Output::write(bool output)
{
    // TODO: Write to pin
}