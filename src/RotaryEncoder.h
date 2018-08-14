#ifndef __RotaryEncoder_h
#define __RotaryEncoder_h

#include "Button.h"
#include <time.h>

#define CLK 1
#define DT 2

class RotaryEncoder
{
private:
    Button* clk;
    Button* dt;
    int firstSignal = 0; // CLK or DT
    int lastCLKState = 0;


    time_t lastTimeOfCLKSignal = 0;
    int lastValueOfCLKSignal = 0;

    int clkSignalModifier = 0;

public:
    RotaryEncoder(unsigned char clkPin, unsigned char dtPin);
    RotaryEncoder(Button* clk, Button* dt);
    ~RotaryEncoder();

    // Check if increment or decrement happen
    // 0 - nothing
    // 1 - ++
    // -1 - --
    int check();
    
};

#endif