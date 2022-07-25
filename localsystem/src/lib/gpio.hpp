#ifndef LEADCLOUDDDRIVER_GPIO_H
#define LEADCLOUDDDRIVER_GPIO_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>

#define DEBUGCROSS

/////////////////////////
//////OFFSETS////////////
//////////////////////////////////////////////////
#define BCM2711_PERI_BASE_ADDR          0xFE000000

#define GPIO_REG_OFFSET                 0x200000
#define COUNTER_1MHz_REGISTER_OFFSET    0x3000

#define GPIO_PWM_BASE_OFFSET            (GPIO_REG_OFFSET + 0xC000)
#define GPIO_CLK_BASE_OFFSET            0x101000

#define REGISTER_BLOCK_SIZE             4*1024

#define PIN_SET_INPUT(x)                *(s_gpio_registers + (x/10)) &= ~(7ull<<(((x)%10)*3))
#define PIN_SET_OUTPUT(x)               *(s_gpio_registers + (x/10)) |= (1ull<<(((x)%10)*3))
//#define PIN_SET_OUTPUT(x)               PIN_SET_INPUT(x); *(s_gpio_registers + (x/10)) |= (1ull<<(((x)%10)*3))
                                            //  x/10    -> 10 gpio pin definitions per GPFSELX
                                            //  & / |   -> clear / set pins
                                            //  % 10    -> subindex to get actual 3 bit register
                                            //  * 3     -> 3 bits per register

                                            //calls set input to clear the bits first

#define PIN_OUTPUT_HIGH(x)              *(s_gpio_registers + (0x1c / sizeof(uint32_t))) = (x)
#define PIN_OUTPUT_LOW(x)               *(s_gpio_registers + (0x28 / sizeof(uint32_t))) = (x)

//////////////////////////////////////////////////


/////////////////////////
//////CLOCK//////////////
//////////////////////////////////////////////////

#define EMPIRICAL_NANOSLEEP_OVERHEAD_US 12
#define MINIMUM_NANOSLEEP_TIME_US 5

/////////////////////////////////////////////////
typedef uint32_t gpio_pins_t;

struct PinMapping {
    int OE; //output enable
    int CLK; // clock
    int STB; //strobe / latch

    int A, B, C, D; //row addressing

    int R1, G1, B1; //first block
    int R2, G2, B2; //second block
};

enum PinConfigEnum {AdafruitHat, AdafruitHatPWMHack};


class GPIO
{
private:
    int slowdown;

    uint32_t *s_gpio_registers;
    uint32_t *s_timer_registers;
    uint32_t *s_PWM_registers;
    uint32_t *s_CLK_registers;

    uint32_t *s_gpio_output_high;
    uint32_t *s_gpio_output_low;
    uint32_t *s_gpio_read;

    PinConfigEnum pinConfig;
    PinMapping p;

    #define DISPLAY_TIME_OFFSET 0
    #define DISPLAY_TIME_BASE 20 //* 1000 * 100 * 3
    #define DISPLAY_TIME_SHIFT 1
    /*long time_table[8] = {
        DISPLAY_TIME_BASE << (0 + DISPLAY_TIME_SHIFT), //0000 0001 //LSb
        DISPLAY_TIME_BASE << (1 + DISPLAY_TIME_SHIFT), //0000 0010
        DISPLAY_TIME_BASE << (2 + DISPLAY_TIME_SHIFT), //0000 0100
        DISPLAY_TIME_BASE << (3 + DISPLAY_TIME_SHIFT), //0000 1000
        DISPLAY_TIME_BASE << (4 + DISPLAY_TIME_SHIFT), //0001 0000
        DISPLAY_TIME_BASE << (5 + DISPLAY_TIME_SHIFT), //0010 0000
        DISPLAY_TIME_BASE << (6 + DISPLAY_TIME_SHIFT), //0100 0000
        DISPLAY_TIME_BASE << (7 + DISPLAY_TIME_SHIFT), //1000 0000 //MSb
        };*/

    long time_table[8] = {
        (DISPLAY_TIME_OFFSET + DISPLAY_TIME_BASE) << (0 * DISPLAY_TIME_SHIFT), //0000 0001 //LSb
        (DISPLAY_TIME_OFFSET + DISPLAY_TIME_BASE) << (1 * DISPLAY_TIME_SHIFT), //0000 0010
        (DISPLAY_TIME_OFFSET + DISPLAY_TIME_BASE) << (2 * DISPLAY_TIME_SHIFT), //0000 0100
        (DISPLAY_TIME_OFFSET + DISPLAY_TIME_BASE) << (3 * DISPLAY_TIME_SHIFT), //0000 1000
        (DISPLAY_TIME_OFFSET + DISPLAY_TIME_BASE) << (4 * DISPLAY_TIME_SHIFT), //0001 0000
        (DISPLAY_TIME_OFFSET + DISPLAY_TIME_BASE) << (5 * DISPLAY_TIME_SHIFT), //0010 0000
        (DISPLAY_TIME_OFFSET + DISPLAY_TIME_BASE) << (6 * DISPLAY_TIME_SHIFT), //0100 0000
        (DISPLAY_TIME_OFFSET + DISPLAY_TIME_BASE) << (7 * DISPLAY_TIME_SHIFT), //1000 0000 //MSb
        };



    //actually writes to registers 
    inline void WriteSetBits(gpio_pins_t value){
        *s_gpio_output_high = static_cast<uint32_t>(value & 0xFFFFFFFF);
    }
    inline void WriteClrBits(gpio_pins_t value){
        *s_gpio_output_low = static_cast<uint32_t>(value & 0xFFFFFFFF);
    }

    uint32_t *map_register(off_t reg_off);
    bool map_all_registers();

    void definePinFuncs();

    
public:

    friend class MatrixDriver;

    GPIO(PinConfigEnum pinConfig, int slowdown);
    ~GPIO();

    bool Init();

    //bool ReconfigureDisplaySettings();

    uint32_t generateRGBCLKMask(){
        return (1 << p.R1 | 1 << p.G1 | 1 << p.B1 | 1 << p.R2 | 1 << p.G2 | 1 << p.B2 | 1 << p.CLK);
    }

    uint32_t generateAddressMask(){
        return (1 << p.A | 1 << p.B | 1 << p.C | 1 << p.D);
    }

    //output manipulation
    inline void SetBits(gpio_pins_t value){
        if (!value) return;
            WriteSetBits(value);
        for (int i = 0; i < slowdown; ++i) {
            WriteSetBits(value);
        }
    }

    inline void ClearBits(gpio_pins_t value){
        if (!value) return;
            WriteClrBits(value);
        for (int i = 0; i < slowdown; ++i)
            WriteClrBits(value);
    }
    
    inline void WriteMaskedBits(gpio_pins_t value, gpio_pins_t mask) {
        ClearBits(~value & mask);
        SetBits(value & mask);
    }

    void SendPulse(int depth){
        SetBits(1 << p.OE);
        wait_nanoseconds(time_table[depth]);
        ClearBits(1 << p.OE);
    }

    void wait_nanoseconds(long nanos) {
        static long kJitterAllowanceNanos = (EMPIRICAL_NANOSLEEP_OVERHEAD_US + 10) * 1000;
        if (nanos > kJitterAllowanceNanos + MINIMUM_NANOSLEEP_TIME_US*1000){
            const uint32_t before = *s_timer_registers;
            struct timespec sleep_time{0, nanos - kJitterAllowanceNanos};
            nanosleep(&sleep_time, nullptr);
            const uint32_t after = *s_timer_registers;
            const long nanoseconds_passed = 1000 * (uint32_t)(after - before);
            if(nanoseconds_passed > nanos){
                return;
            } else {
                nanos -= nanoseconds_passed;
            }
        } else {
            if(nanos > (EMPIRICAL_NANOSLEEP_OVERHEAD_US + MINIMUM_NANOSLEEP_TIME_US) * 1000){
                struct timespec sleep_time = {0, nanos - EMPIRICAL_NANOSLEEP_OVERHEAD_US*1000};
                nanosleep(&sleep_time, nullptr);
                return;
            }
        }

        busy_wait(nanos);
    }

    void busy_wait(long nanos) {
        if (nanos < 20) return;
        // Interesting, the Pi4 is _slower_ than the Pi3 ? At least for this busy loop
        for (uint32_t i = (nanos - 5) * 100 / 132; i != 0; --i) {
            asm("");
        }
    }
};


#endif //LEADCLOUDDDRIVER_GPIO_H
