#include "Button.h"

Button::Button(unsigned char pin)
{
    input = new Input(pin);
}

Button::Button(Input* input)
{
    this->input = input;
}

Button::~Button()
{
    delete input;
}

int Button::getEventUp()
{
    int state = input->read();
    // Kombination of XOR of state and last State and Kombination of XOR and state
    // lead to 1 if Event up occurs
    int event = (state ^ lastState) & state;
    lastState = state;
    return event;
}

int Button::getEventDown()
{
    int state = input->read();
    // Kombination of XOR of state and last State and Kombination of XOR and lastState
    // lead to 1 if Event down occurs
    int event = (state ^ lastState) & lastState; 
    lastState = state;
    return event;    
}

int Button::getEvent()
{
    int state = input->read();
    int event = (state ^ lastState);
    if(event & state)
    {
        lastState = state;
        return 1;
    }
    else if(event & lastState)
    {
        lastState = state;
        return 2;
    }
}

int Button::read()
{
    return this->input->read();
}