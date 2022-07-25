#ifndef LEADCLOUDDDRIVER_MATRIXDRIVER_H
#define LEADCLOUDDDRIVER_MATRIXDRIVER_H

#include "gpio.hpp"

#define DEBUGCROSS

class MatrixDriver
{
private:
    GPIO gpio;
    uint32_t* pixelBuffer;

    uint32_t* rowBuffer;
    uint32_t* bitBuffer;

    uint32_t color_clk_mask;
    uint32_t address_mask;
    
    int row_n, col_n, color_depth;
public:
    MatrixDriver();
    ~MatrixDriver();

    bool Init(uint32_t* pixelBuffer, int row_n, int col_n);
    void GenerateMasks();

    inline void generateRow(int row);
    inline void generateRow2(int row);
    inline void generateBitMatrix();
    inline uint32_t RGBA8tobitMapping(uint32_t color1, uint32_t color2, int bit);
    inline uint32_t set_address(int address);

    uint32_t* getBuffer();
    int getRowN();
    int getColN();

    GPIO* getGPIO(){
        return &gpio;
    }

    void driveMatrix();
    void driveMatrix2();
    void driveRow();
};


#endif