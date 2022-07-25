#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include <time.h>

#include "../Drawer.hpp"

int main(int argc, char **arg)
{
    if (argc < 4)
    {
        printf("Pong usage: <pong> <pixelBufferPath> <row_n> <col_n>");
        return -1;
    }
    long row_n = strtol(arg[2], NULL, 10);
    long col_n = strtol(arg[3], NULL, 10);

    int pixelBufferFd = open(arg[1], O_RDWR);
    if (pixelBufferFd < 0)
        return -1;

    uint32_t *pixelBuffer = (uint32_t *)mmap(NULL, row_n * col_n * sizeof(uint32_t), PROT_WRITE, MAP_SHARED, pixelBufferFd, 0);
    if (pixelBuffer == MAP_FAILED)
        return -1;

    close(pixelBufferFd);

    printf("Playing pong...");

    time_t t;
    srand((unsigned)time(&t));

    Drawer d(pixelBuffer, row_n, col_n);

    const int padLen = 8;
    int leftX = (row_n / 2 - 1) - padLen / 2; // pads start in the middle;
    int leftY = 0;

    int rightX = leftX; // reuse val
    int rightY = col_n - 1;

    int ballX = row_n / 2 - 1;
    int ballY = col_n / 2 - 1;
    int ball_rdx = rand() % 7 + 1;     // reverse dx
    int ballXDir = rand() % 2 * 2 - 1; // up or down (1 or -1)

    int ball_i_rdx = 0;
    int ball_rdy = 1; // always moves 1 to left/right
    int ball_i_rdy = 0;
    int ballYDir = rand() % 2 * 2 - 1;

    // printf("Row_n = %d, Col_n = %d\n", row_n, col_n);

    while (true)
    {
        // update positions
        ball_i_rdx = (ball_i_rdx + 1) % ball_rdx;
        if (ball_i_rdx == ball_rdx - 1)
        {
            ballX += ballXDir;
        }

        ball_i_rdy = (ball_i_rdy + 1) % ball_rdy;
        if (ball_i_rdy == ball_rdy - 1)
        {
            ballY += ballYDir;
        }

        // can be better optimized
        // it will always be moving so just move it to either the right or left from ballYDir
        if (ballYDir < 0)
        {
            if (leftX > ballX - padLen / 2 && (leftX > 0))
            {
                leftX--;
            }
            else if (leftX < ballX - padLen / 2 && (leftX + padLen < row_n - 1))
            {
                leftX++;
            }
        }
        else
        {
            if (rightX > ballX - padLen / 2 && rightX > 0)
            {
                rightX--;
            }
            else if (rightX < ballX - padLen / 2 && (rightX + padLen < row_n - 1))
            {
                rightX++;
            }
        }

        // check for collision

        // check for top and bottom collision
        if (ballX <= 0)
        { // simply invert direction
            ballXDir = 1;
        }
        else if (ballX >= row_n - 1)
        {
            ballXDir = -1;
        }
        // check for pad collision
        if (ballY <= 0 || ballY >= col_n)
        { // out of bounds, not expected to happen
            ballX = row_n / 2 - 1;
            ballY = col_n / 2 - 1;
        }

        if (ballY == leftY + 1 && (ballX >= leftX || ballX <= leftX + padLen))
        { // hit left pad
            ballYDir = -ballYDir;
        }
        else if (ballY == rightY - 1 && (ballX >= rightX || ballX <= rightX + padLen))
        { // hit right pad
            ballYDir = -ballYDir;
        }

        // draw pads and ball
        // d.fill(0xFF0000FF); //red
        d.fill(0);
        d.pixel(0xFFFFFFFF, ballX, ballY);

        d.fastVLine(0xFFFFFFFF, leftX, leftX + padLen, leftY);
        d.fastVLine(0xFFFFFFFF, rightX, rightX + padLen, rightY);

        // printf("Ball x:%d y:%d\n", ballX, ballY);
        // wait for some time
        timespec refreshTime = {0, (long)1e9 / 30 - 1000}; // refresh
        nanosleep(&refreshTime, NULL);
    }
}