#include "gpio.hpp"

PinMapping AdafruitHatPWMPinMapping = {
        .OE = 18, //PWM hack
        .CLK = 17,
        .STB = 21,

        .A = 22,
        .B = 26,
        .C = 27,
        .D = 20,

        .R1 = 5,
        .G1 = 13,
        .B1 = 6,

        .R2 = 12,
        .G2 = 16,
        .B2 = 23,

};

PinMapping AdafruitHatPinMapping = {
        .OE = 4, 
        .CLK = 17,
        .STB = 21,

        .A = 22,
        .B = 26,
        .C = 27,
        .D = 20,

        .R1 = 5,
        .G1 = 13,
        .B1 = 6,

        .R2 = 12,
        .G2 = 16,
        .B2 = 23,

};

GPIO::GPIO(PinConfigEnum pinConfig, int slowdown)
{
    this->pinConfig = pinConfig;
    this->slowdown = slowdown;
}

GPIO::~GPIO()
{
    if(s_gpio_registers){ //!= nullptr
        munmap(s_gpio_registers, REGISTER_BLOCK_SIZE);
    }

    if(s_timer_registers){
        munmap(s_timer_registers - 1, REGISTER_BLOCK_SIZE);
    }

    if(s_PWM_registers){
        munmap(s_PWM_registers, REGISTER_BLOCK_SIZE);
    }

    if(s_CLK_registers){
        munmap(s_CLK_registers, REGISTER_BLOCK_SIZE);
    }
}


uint32_t *GPIO::map_register(off_t reg_off) {
    int mem_fd = open("/dev/mem", O_RDWR | O_SYNC);

    if (mem_fd < 0)
        return nullptr;

    auto *result = (uint32_t *) mmap(
            nullptr,
            REGISTER_BLOCK_SIZE,
            PROT_READ | PROT_WRITE,
            MAP_SHARED,
            mem_fd,
            BCM2711_PERI_BASE_ADDR + reg_off
            );

    close(mem_fd);

    if(result == MAP_FAILED){
        perror("mmap error: ");
        fprintf(stderr, "MMapping from base 0x%x, offset 0x%lx\n", BCM2711_PERI_BASE_ADDR, reg_off);
        return nullptr;
    }

    return result;
}

bool GPIO::map_all_registers(){
    printf("\tMapping registers...\n");
    if(s_gpio_registers != nullptr)
        return true;

    s_gpio_registers = map_register(GPIO_REG_OFFSET);
    if(s_gpio_registers == nullptr)
        return false; //could not map gpio registers

    printf("\t\tMapped s_gpio_registers\n");

    uint32_t * timer_reg = map_register(COUNTER_1MHz_REGISTER_OFFSET);
    if(timer_reg != nullptr){
        s_timer_registers = timer_reg + 1;
        printf("\t\tSuccessfully mapped timer\n");
    }

    s_PWM_registers = map_register(GPIO_PWM_BASE_OFFSET);
    printf("\t\tMapped s_PWM_registers\n");
    s_CLK_registers = map_register(GPIO_CLK_BASE_OFFSET);
    printf("\t\tMapped s_CLK_registers\n");

    s_gpio_output_high = s_gpio_registers + (0x1c / sizeof(uint32_t));
    s_gpio_output_low = s_gpio_registers + (0x28 / sizeof(uint32_t));
    s_gpio_read = s_gpio_registers + (0x34 / sizeof(uint32_t));

    return true;
}

void GPIO::definePinFuncs() {
    PIN_SET_INPUT(p.OE);
    PIN_SET_INPUT(p.CLK);
    PIN_SET_INPUT(p.STB);

    PIN_SET_INPUT(p.A);
    PIN_SET_INPUT(p.B);
    PIN_SET_INPUT(p.C);
    PIN_SET_INPUT(p.D);

    PIN_SET_INPUT(p.R1);
    PIN_SET_INPUT(p.G1);
    PIN_SET_INPUT(p.B1);

    PIN_SET_INPUT(p.R2);
    PIN_SET_INPUT(p.G2);
    PIN_SET_INPUT(p.B2);

    //clear to set

    PIN_SET_OUTPUT(p.OE);
    PIN_SET_OUTPUT(p.CLK);
    PIN_SET_OUTPUT(p.STB);

    PIN_SET_OUTPUT(p.A);
    PIN_SET_OUTPUT(p.B);
    PIN_SET_OUTPUT(p.C);
    PIN_SET_OUTPUT(p.D);

    PIN_SET_OUTPUT(p.R1);
    PIN_SET_OUTPUT(p.G1);
    PIN_SET_OUTPUT(p.B1);

    PIN_SET_OUTPUT(p.R2);
    PIN_SET_OUTPUT(p.G2);
    PIN_SET_OUTPUT(p.B2);
}

bool GPIO::Init(){
    printf("Initializing matrix...\n");

    switch (pinConfig)
    {
    case AdafruitHat:
        p = AdafruitHatPinMapping;
        break;
    case AdafruitHatPWMHack:
        p = AdafruitHatPWMPinMapping;
        break;
    default:
        printf("Unrecognized Pin mapping, Init failed\n");
        return false;
        break;
    }

    s_gpio_registers = nullptr;
    s_PWM_registers = nullptr;
    s_timer_registers = nullptr;
    s_CLK_registers = NULL;


    if(map_all_registers()){
        printf("\tSuccessfully mapped all registers\n");
    } else{
        printf("\tFailed to map registers\n");
        return false;
    }

    definePinFuncs();
    printf("Set GPIO functions\n");

    return true;
}