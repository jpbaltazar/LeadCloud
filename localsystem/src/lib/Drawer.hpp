#ifndef LEADCLOUDDDRIVER_DRAWER_H
#define LEADCLOUDDDRIVER_DRAWER_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <string>

#define ASCII_LETTERS_H
#define ASCII_START 32 //space
#define ASCII_STOP 127 //~
#define ASCII_SPACING 1
#define ASCII_WIDTH (8 + ASCII_SPACING)
#define ASCII_HEIGHT 13
extern unsigned char letterTable[95][13];

class Drawer
{
private:
    uint32_t* buffer;
    int row_n, col_n;
public:
    Drawer(uint32_t* bufferPointer, int row_n, int col_n);
    ~Drawer();

    void zero();
    void fill(uint32_t color);

    void line(uint32_t color, int x1, int y1, int x2, int y2);
    void fastHLine(uint32_t color, int x, int y1, int y2);
    void fastVLine(uint32_t color, int x1, int x2, int y);
    void pixel(uint32_t color, int x, int y);

    void drawTextSmall(int x, int y, const char* str, uint32_t color, uint32_t fillColor); //7x13
    void drawTextBig(int x, int y, const char* str, uint32_t color, uint32_t fillColor); //14x26

    void rainbowFill();

    void rainbowFillIterativeScroll();
    void rainbowFillIterative();


    void rainbowTriangle();
    void rainbowTriangle2();

    void ColorTest();

    uint32_t getPixel(int x, int y){
        return buffer[x * col_n + y];
    }
};





#endif