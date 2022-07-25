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
    printf("HGEEEEEEEEEEYYYYYYYYY\n");

    if (argc < 4)
    { // 5
        printf("scrollText usage: <scrollText> <pixelBufferPath> <row_n> <col_n> (<text to display>) (<color>)");
        return -1;
    }

    printf("Trying args...\n");

    for (int i = 0; i < argc; i++)
    {
        printf("\tArg %d: %s\n", i, arg[i]);
    }

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

    Drawer d(pixelBuffer, row_n, col_n);

    const char *text = argc > 4 ? arg[4] : "Scrolling text! ";
    uint32_t color = argc > 5 ? strtol(arg[5], NULL, 16) : 0xFF0000FF;

    int len = (int)strlen(text);
    printf("Now scrolling text: \"%s\"", text);

    for (int i = 0;; i = (i + 1) % (ASCII_WIDTH * len))
    {
        d.fill(0);

        d.drawTextSmall(row_n / 2 - ASCII_HEIGHT / 2, -i, text, color, 0);
        d.drawTextSmall(row_n / 2 - ASCII_HEIGHT / 2, ASCII_WIDTH * len - i, text, color, 0);

        // wait for some time
        timespec refreshTime = {0, (long)1e9 / 30 - 1000}; // refresh
        nanosleep(&refreshTime, NULL);
    }
}