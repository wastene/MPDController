#include "LCD.h"
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

LCD::LCD(unsigned int addr)
{
    this->addr = addr;
    dev = I2C("/dev/i2c-1", addr);
    init();
}

LCD::LCD(unsigned int addr, unsigned int baudrate)
{
    this->addr = addr;
    dev = I2C("/dev/i2c-1", addr);
    init();
}

void LCD::init()
{
    write4Bits(0x03 << 4);
    usleep(4500);
    write4Bits(0x03 << 4);
    usleep(4500);
    write4Bits(0x03 << 4);
    usleep(150);

    write4Bits(0x02  << 4);

    write(0x28, CMD);
    write(0x0c, CMD);
    clear();
    write(0x06, CMD);
    write(0x02, CMD);
    usleep(2000);
}

void LCD::reinit()
{
    dev.open("/dev/i2c-1", this->addr);
    init();
}

void LCD::write(unsigned char data, int mode)
{
    write4Bits(mode | data & 0xF0);
    write4Bits(mode | (data << 4) & 0xF0);
}

void LCD::write4Bits(unsigned char data)
{
    dev.write(data | ports.BACKLIGHT);
    dev.write(data | ports.E | ports.BACKLIGHT);
    usleep(1);
    dev.write((data & ~ports.E) | ports.BACKLIGHT);
    usleep(50);
}

void LCD::on()
{
    ports.BACKLIGHT = (unsigned char)0x08;
    write(0x0C, CMD);
}

void LCD::off()
{
    ports.BACKLIGHT = 0x00;
    write(CMD_DISPLAYCONTROL | CMD_DISPLAYOFF, CMD);
}

void LCD::clear()
{
    write(CMD_CLEAR, CMD);
    usleep(2000);
}

void LCD::clear(unsigned char line)
{
    write(LINES[line], CMD);
    for(int i=0; i<16; i++){
        // 0x20 => Space
        write(0x20, CHR);
    }
}

void LCD::write(unsigned char position, const char* text, unsigned char line)
{
    write(LINES[line]+position, CMD);
    for(int i=0; i<16-position; i++){
        if(text[i] == '\0'){
            break;
        }
        write(text[i], CHR);
    }
}

void LCD::writeLine(const char* text, unsigned char line)
{
    write(LINES[line], CMD);

    int i = 0;
    for(i=0; i<16; i++){
        if(text[i] == '\0'){
            break;
        }
        write(text[i], CHR);
    }
    for(;i<16; i++){
        write(0x20, CHR);
    }
}

void LCD::centralWrite(const char* text, unsigned char length, unsigned char line)
{
    write((unsigned char)((16-length)/2), text, line);
}

void callback(int signum)
{
    
}

void LCD::scrollBegin(const char* text, unsigned char line)
{
    // Initialization
    scroll[line].activated = true;
    strcpy(scroll[line].text, text);
    scroll[line].position = 0;

    // TODO: Do Something to get interrupt of 750ms
    // Do from outside - Main should count

    // Initialization of Timer interrupt
    //signal(SIGINT, &callback);
    //ualarm(750000, 750000);

    writeLine(text, line);
}

void LCD::scrollCallback()
{
    for(int i=0; i<2; i++)
    {
        if(scroll[i].activated)
        {
            if(scroll[i].position >= 15)
            {
                scroll[i].position = 0;
            }
            else
            { 
                scroll[i].position++;
            }

            char text[16];
            char* movedPointer = scroll[i].text+scroll[i].position;
            strncpy(text, movedPointer, 16);

            int begin = -1;
            for(int j=0; j<16; j++)
            {
                if(text[j] == '\0' && begin == -1)
                {
                    begin = j;
                    text[j] == 0x20;
                }
                else if(text[j] == '\0' && begin >= 0)
                {
                    if(j-begin == 0)
                    {
                        // Insert ' *** ' between end of text and beginning
                        text[j] = ' ';
                        text[j+1] = '*';
                        text[j+2] = '*';
                        text[j+3] = '*';
                        text[j+4] = ' ';
                    }

                    if(j-begin > 5)
                    {
                        // Insert at end text from beginning to produce a circuit
                        for(int k=j, l=0; k<16-j; k++, l++)
                        {
                            text[k] = scroll[i].text[l];
                        }
                        break;
                    }
                }
            }
            // Write Line
            writeLine(text, i);
        }
    }
}

void LCD::scrollEnd(unsigned char line)
{
    scroll[line].activated = false;
}


void LCD::scrollLeft()
{
    // CURSORSHIFT | DISPLAYMOVE | MOVELEFT
    write(CMD_CURSORSHIFT | 0x08, CMD);
}

void LCD::scrollRight()
{
    // CURSORSHIFT | DISPLAYMOVE | MOVERIGHT
    write(CMD_CURSORSHIFT | 0x0c, CMD);
}
