#ifndef __Button_h
#define __Button_h

#include "Input.h"

#define EVENT_UP 1
#define EVENT_DOWN 2

class Button
{
private:
    Input *input;
    int lastState = 0;

public:
    Button(unsigned char);
    Button(Input* input);
    ~Button();

    // Get Event of Button
    int getEventUp();
    int getEventDown();

    // 1 Up - 2 Down
    int getEvent();

    // Methods from Input
    int read();
};

#endif