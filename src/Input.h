#ifndef __Input_h
#define __Input_h

class Input
{
private:
    unsigned char pin;
    int file;

public:
    Input(unsigned char pin);
    Input(const Input &input);
    ~Input();

    int read();
};

#endif