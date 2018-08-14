#ifndef __LCD_H
#define __LCD_H

#include "I2C.h"

// Ports
#define CHR 1
#define CMD 0

// CMDs:
#define CMD_CLEAR 0x01
#define CMD_HOME 0x02
#define CMD_ENTRYMODESET 0x04
#define CMD_DISPLAYCONTROL 0x08
#define CMD_CURSORSHIFT 0x10
#define CMD_FUNCTIONSET 0x20
#define CMD_SETCGRAMADDR 0x40
#define CMD_SETDDRAMADDR 0x80

#define CMD_DISPLAYON 0x04
#define CMD_DISPLAYOFF 0x00

typedef struct Ports {
    unsigned char RS = 0x01; 
    unsigned char E = 0x04;
    unsigned char BACKLIGHT = 0x08;
    unsigned char RW = 0x20;
} Ports_t;

typedef struct ScrollText {
    unsigned char activated = true;
    char* text = '\0';
    unsigned char position = 0;
} ScrollText_t;

class LCD 
{
private:
    unsigned int addr = 0x27;
    I2C dev;
    Ports_t ports;
    const unsigned char LINES[2] {0x80, 0xC0};
    const unsigned char INIT[8] {0x03, 0x03, 0x03, 0x02, 0x28, 0x0c, 0x01, 0x06};
    ScrollText_t scroll[2];

public:
    LCD(unsigned int addr);
    LCD(unsigned int addr, unsigned int baudrate);

    void reinit();

    // Turn LCD Display on
    void on();

    // Turn LCD Display off
    void off();

    // Clears complete LCD Display
    void clear();

    // Clears only specific line of LCD Display
    void clear(unsigned char line);

    // Write to LCD Display to line (beginning at 0) beginning at position (beginning at 0)
    // text need to be zero terminated
    void write(unsigned char position, const char* text, unsigned char line);

    // Write Text to LCD Display beginning at Position 0 and fills Rest with blank spaces
    // text need to be zero terminated
    void writeLine(const char* text, unsigned char line);

    // Write Text to Central of LCD Display to specific line
    // text need to be zero terminated
    void centralWrite(const char* text, unsigned char length, unsigned char line);

    // Scroll Text in specific line to left
    // text need to be zero terminated
    void scrollBegin(const char* text, unsigned char line);

    void scrollCallback();

    // End scrolling of Text in specific line
    void scrollEnd(unsigned char line);

    // Scroll whole display to left
    void scrollLeft();

    // Scroll whole display to right
    void scrollRight();

private:
    void init();
    void write(unsigned char, int);
    void write4Bits(unsigned char);
};

#endif