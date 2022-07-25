#include "MatrixDriver.hpp"

MatrixDriver::MatrixDriver() : gpio(AdafruitHat, 4) //4
{
    pixelBuffer = nullptr;
    color_clk_mask = 0;
    address_mask = 0;
}

MatrixDriver::~MatrixDriver()
{

}

bool MatrixDriver::Init(uint32_t* pixelBuffer, int row_n, int col_n){
    if(row_n < 0 || col_n < 0){
        printf("Forbidden matrix dimensions %dx%d\n", row_n, col_n);
        return false;
    }
    this->row_n = row_n;
    this->col_n = col_n;
    this->color_depth = 8;

    printf("Started matrix with dimensions %dx%d\n", row_n, col_n);

    if(!gpio.Init())
        return false;

    this->pixelBuffer = pixelBuffer;
    if(pixelBuffer == nullptr)
        return false;

    rowBuffer = static_cast<uint32_t *>(calloc(col_n * color_depth, sizeof(uint32_t)));
    if(rowBuffer != nullptr){
        printf("\tSuccessfully allocated row buffer\n");
    }
    else{
        printf("\tFailed to allocate row buffer\n");
        return false;
    }

    bitBuffer = static_cast<uint32_t *>(calloc(row_n * col_n * color_depth, sizeof(uint32_t)));
    if(bitBuffer != nullptr){
        printf("\tSuccessfully allocated bit buffer\n");
    }
    else{
        printf("\tFailed to allocate bit buffer\n");
        return false;
    }

#ifdef DEBUGCROSS    

    for(int i = 0; i < row_n * col_n; i++){
        pixelBuffer[i] = 0;//0x00FF00FF;
    }
    
    //Red cross
    for (int i = 0; i < row_n; ++i) {
        //Left corner
        pixelBuffer[col_n * i + 2 * i] = 0xFF0000FF;
        pixelBuffer[col_n * i + 2 * i + 1] = 0xFF0000FF;
        //Right corner
        pixelBuffer[col_n * i + 63 - 2 * i] = 0xFF0000FF;
        pixelBuffer[col_n * i + 63 - 2 * i - 1] = 0xFF0000FF;
    }

    /*for(int i = 0; i < col_n; i++){
        pixelBuffer[i] = 0xFF | ((i * 4) << 24);
    }
    //colors are shit*/

    printf("Debug cross written.\n");
#endif
    GenerateMasks();

    return true;
}

void MatrixDriver::GenerateMasks(){
    color_clk_mask = gpio.generateRGBCLKMask();
    address_mask = gpio.generateAddressMask();

}

void MatrixDriver::driveMatrix(){
    //static int address = -1;
    //address++;
    //address &= 0x0F;
    //for(int row = address; row == address; row++){
    for(int row = 0; row < row_n / 2; row++){
        generateRow(row);
        uint32_t address_bits = set_address(row);

        uint32_t* row_p = rowBuffer;
        for(int depth = 0; depth < color_depth; depth++){
            for(int col = 0; col < col_n; col++){
                gpio.WriteMaskedBits(*row_p, color_clk_mask);
                gpio.SetBits(1 << gpio.p.CLK);
                
                row_p++;
            }
            //gpio.wait_nanoseconds(40);
            gpio.ClearBits(color_clk_mask);

            gpio.WriteMaskedBits(address_bits, address_mask);

            //gpio.SetBits(1 << gpio.p.OE);

            gpio.SetBits(1 << gpio.p.STB);
            gpio.ClearBits(1 << gpio.p.STB);

            gpio.SendPulse(depth);
        }
    }
}

void MatrixDriver::driveMatrix2(){
    generateBitMatrix();
    for(int row = 0; row < row_n / 2; row++){
        uint32_t* row_p = bitBuffer + row * col_n * color_depth;
        uint32_t address_bits = set_address(row);

        for(int depth = 0; depth < color_depth; depth++){
            for(int col = 0; col < col_n; col++){
                gpio.WriteMaskedBits(*row_p, color_clk_mask);
                gpio.SetBits(1 << gpio.p.CLK);
                
                row_p++;
            }
            //gpio.wait_nanoseconds(40);
            gpio.ClearBits(color_clk_mask);

            gpio.WriteMaskedBits(address_bits, address_mask);

            //gpio.SetBits(1 << gpio.p.OE);

            gpio.SetBits(1 << gpio.p.STB);
            gpio.ClearBits(1 << gpio.p.STB);

            gpio.SendPulse(depth);
        }
        gpio.SetBits(1 << gpio.p.OE); //prevents a bright last row

    }

}

void MatrixDriver::driveRow(){
    static int row = -1;
    row++;
    row &= 0x0F;

    generateRow(row);
    uint32_t address_bits = set_address(row);

    uint32_t* row_p = rowBuffer;
    for(int depth = 0; depth < color_depth; depth++){
        for(int col = 0; col < col_n; col++){
            gpio.WriteMaskedBits(*row_p, color_clk_mask);
            gpio.SetBits(1 << gpio.p.CLK);
            
            row_p++;
        }
        //gpio.wait_nanoseconds(40);
        gpio.ClearBits(color_clk_mask);

        gpio.WriteMaskedBits(address_bits, address_mask);

        //gpio.SetBits(1 << gpio.p.OE);

        gpio.SetBits(1 << gpio.p.STB);
        gpio.ClearBits(1 << gpio.p.STB);

        gpio.SendPulse(depth);
    }
    
}

void MatrixDriver::generateBitMatrix(){
    for(int row = 0; row < row_n/2; row++){
        uint32_t* row_PP = bitBuffer + row * col_n * color_depth;

        for(int col = 0; col < col_n; col++){
            uint32_t color1 = pixelBuffer[row * col_n + col];
            uint32_t color2 = pixelBuffer[(row + row_n/2) * col_n + col];

            uint32_t* row_p = row_PP + col;
            
            for(int depth = 0; depth < color_depth; depth++, row_p += col_n){
                *row_p = (((color1 >> 24) >> depth) & 0x1) << gpio.p.R1;
                *row_p |= (((color1 >> 16) >> depth) & 0x1) << gpio.p.G1;
                *row_p |= (((color1 >> 8) >> depth) & 0x1) << gpio.p.B1;

                *row_p |= (((color2 >> 24) >> depth) & 0x1) << gpio.p.R2;
                *row_p |= (((color2 >> 16) >> depth) & 0x1) << gpio.p.G2;
                *row_p |= (((color2 >> 8) >> depth) & 0x1) << gpio.p.B2;
            }
        }
    }
}

//depth/col/col/.../col/depth -> better for sequential reads (physical proximity)
// column 0, depth 0 | column 1, depth 0|...|column 0 , depth 1|...
void MatrixDriver::generateRow(int row){ 
    for(int col = 0; col < col_n; col++){
        uint32_t color1 = pixelBuffer[row * col_n + col];
        uint32_t color2 = pixelBuffer[(row + row_n/2) * col_n + col];

        uint32_t* row_p = rowBuffer + col;
        for(int depth = 0; depth < color_depth; depth++, row_p += col_n){
            *row_p = (((color1 >> 24) >> depth) & 0x1) << gpio.p.R1;
            *row_p |= (((color1 >> 16) >> depth) & 0x1) << gpio.p.G1;
            *row_p |= (((color1 >> 8) >> depth) & 0x1) << gpio.p.B1;

            *row_p |= (((color2 >> 24) >> depth) & 0x1) << gpio.p.R2;
            *row_p |= (((color2 >> 16) >> depth) & 0x1) << gpio.p.G2;
            *row_p |= (((color2 >> 8) >> depth) & 0x1) << gpio.p.B2;
        }
    }
}

uint32_t MatrixDriver::set_address(int address) {
    uint32_t pins = 0;
    pins |= ((address & 0x1) >> 0) << gpio.p.A; //Abcd
    pins |= ((address & 0x2) >> 1) << gpio.p.B; //aBcd
    pins |= ((address & 0x4) >> 2) << gpio.p.C; //abCd
    pins |= ((address & 0x8) >> 3) << gpio.p.D; //abcD
    
    return pins;
}

uint32_t* MatrixDriver::getBuffer(){
    return pixelBuffer;
}

int MatrixDriver::getRowN(){
    return row_n;
}

int MatrixDriver::getColN(){
    return col_n;
}