#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include <time.h>

#include "../lib/Drawer.hpp"
#include "../Driver/DriverWrapper.hpp"

int main(int argc, char **arg)
{
    if (argc < 2)
    { // 5
        printf("scrollText usage: <showConfigKey> <key>");
        return -1;
    }

    long row_n = 32;
    long col_n = 64; // minimum sizes

    uint32_t pixelBuffer[32 * 64];

    DriverWrapperFactory f;
    DriverWrapper *d = f.CreateDriverWrapper();
    if (d == nullptr)
    {
        return -EBUSY; // no available device driver instance
    }

    d->resize(32, 64);
    int driverFd = d->getFd();

    Drawer drawer(pixelBuffer, row_n, col_n);

    drawer.fill(0);
    drawer.drawTextBig(row_n / 2 - ASCII_HEIGHT, 0, arg[1], 0xFFFFFFFF, 0);

    write(driverFd, pixelBuffer, sizeof(uint32_t) * row_n * col_n);

    return 0; // no need to keep driving it
}