#ifndef __Output_h
#define __Output_h

class Output
{
private:
    unsigned char pin;

public:
    Output(unsigned char pin);
    ~Output();

    int write(bool);
};

#endif