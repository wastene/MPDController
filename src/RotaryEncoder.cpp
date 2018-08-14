#include "RotaryEncoder.h"

RotaryEncoder::RotaryEncoder(unsigned char clkPin, unsigned char dtPin)
{
    clk = new Button(clkPin);
    dt = new Button(dtPin);
}

RotaryEncoder::RotaryEncoder(Button* clk, Button* dt)
{
    this->clk = clk;
    this->dt = dt;
}

RotaryEncoder::~RotaryEncoder()
{
    delete clk;
    delete dt;
}


int RotaryEncoder::check()
{
    /*
    int clkSignal = this->clk->read();
    if(lastTimeOfCLKSignal == 0){
        lastValueOfCLKSignal = clkSignal;
        time(&lastTimeOfCLKSignal);
    }
    else if(lastValueOfCLKSignal != clkSignal)
    {
        time_t currentTime = 0;
        time(&currentTime);

        if(currentTime-lastTimeOfCLKSignal > 1)
        {
            clkSignalModifier = (clkSignal == 0) ? 0 : 1;
        }
    }*/

    int returnValue = 0;
    int clkState = clk->getEventUp(); // UP=1 DOWN=2
    int dtState = dt->getEventUp();

    clkState -= clkSignalModifier;
    if(clkState == 1 && dtState == 0 && firstSignal == 0)
    {
        firstSignal = CLK;
    }
    else if(clkState == 0 && dtState == 1 && firstSignal == 0)
    {
        firstSignal = DT;
    }
    else if(clkState == 0 && dtState == 1 && firstSignal == CLK)
    {
        returnValue = 1;
        firstSignal = 0;
    }
    else if(clkState == 1 && dtState == 0 && firstSignal == DT)
    {
        returnValue = -1;
        firstSignal = 0;
    }

    //lastCLKState = clkState;
    return returnValue;
}