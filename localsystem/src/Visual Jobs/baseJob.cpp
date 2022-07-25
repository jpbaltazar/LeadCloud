#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include <time.h>

#include "../lib/Drawer.hpp"

int main(int argc, char **arg)
{
    if (argc < 4)
    {
        printf("baseJob usage: <HelloFriends> <pixelBufferPath> <row_n> <col_n> <text to display>");
        return -1;
    }

    // printf("Trying args...\n");

    /*for(int i = 0; i < argc; i++){
        printf("\tArg %d: %s\n", i, arg[i]);
    }*/

    long row_n = strtol(arg[2], NULL, 10);
    long col_n = strtol(arg[3], NULL, 10);

    int pixelBufferFd = open(arg[1], O_RDWR);
    if (pixelBufferFd < 0)
        return -1;

    // printf("Args were good\n");

    uint32_t *pixelBuffer = (uint32_t *)mmap(NULL, row_n * col_n * sizeof(uint32_t), PROT_WRITE, MAP_SHARED, pixelBufferFd, 0);
    if (pixelBuffer == MAP_FAILED)
        return -1;

    close(pixelBufferFd);

    // printf("Mapped pixelBuffer\n");

    // init

    printf("Started \"Lead Cloud\" logo\n");

    Drawer d(pixelBuffer, row_n, col_n);

    const char *logo1 = "Lead";
    const char *logo2 = "Cloud";

    int row = 0;

    uint32_t textColor = 0xFFFFFFFF;
    uint32_t barColor = 0xFF00FFFF;
    uint32_t altBarColor = 0xFF00FF;

    int textX = 6;
    int textY = 4;

    // loop
    while (true)
    {
        d.fill(0);

        d.drawTextSmall(textX, textY, logo1, textColor, 0); // while letters
        d.drawTextSmall(textX + 13, textY + 9, logo2, textColor, 0);

        for (int i = 0; i < 10; i++)
        {
            int row_i = row / 6;
            int x = 3 + row_i;
            int y = 4 + i;
            d.pixel(d.getPixel(x + 2, y) != textColor ? barColor : altBarColor, x + 2, y); // purple
            d.pixel(d.getPixel(x + 3, y) != textColor ? barColor : altBarColor, x + 3, y); // purple

            d.pixel(d.getPixel(x + 15, y + 9) != textColor ? barColor : altBarColor, x + 15, y + 9); // purple
            d.pixel(d.getPixel(x + 16, y + 9) != textColor ? barColor : altBarColor, x + 16, y + 9); // purple
        }

        row = (row + 1) % (12 * 6);
        // wait for some time
        timespec refreshTime = {0, (long)1e9 / 30 - 1000}; // refresh
        nanosleep(&refreshTime, NULL);
    }
}