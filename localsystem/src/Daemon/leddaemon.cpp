#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/fs.h>
#include <signal.h>
#include <string.h>
#include <stdint.h>
#include <thread>
#include <errno.h>
#include <stdlib.h>

#include "../lib/ledDefs.h"
#include "../Driver/DriverWrapper.hpp"
#include "../lib/MatrixDriver.hpp"

//#define DEBUG

static int logfd = -1;

#define ONE_SEC_IN_NANO 1000000000

#define FPS 60

uint32_t *pixelBuffer = nullptr;

static int bufferLen = 0; // in bytes

static MatrixDriver *m; // uninitialized

static void sigterm_handler(int signo)
{
    switch (signo) // just to shut the warning up
    {
    case SIGTERM:
        break;

    default:
        return;
        break;
    }

    printf("Got SIGTERM: exiting...\n");

    delete (m);

    if (logfd != -1)
        close(logfd);
}

void sleep_remainder(timespec start_time, timespec sleep_time)
{
    timespec end_time;
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    long nano = (end_time.tv_sec - start_time.tv_sec) * (long)1e9 + (end_time.tv_nsec - start_time.tv_nsec);

    // sleep remainder of what was supposed
    sleep_time.tv_nsec = sleep_time.tv_sec * (long)1e9 + sleep_time.tv_nsec - nano;
    sleep_time.tv_sec = 0;
    /*static bool firstTime = true;
    if(firstTime){
        double durationInSecs = sleep_time.tv_nsec / (1e9);
        double period = 1.0/settings.fps;
        printf("Slept for: %ld ns (%f s) -> still had %f s out of %f\n", sleep_time.tv_nsec, durationInSecs, (period - durationInSecs), period);
        firstTime = false;
    }*/
    nanosleep(&sleep_time, 0);
}

void run(int fd)
{
    // b -> before
    // a -> after
    // r -> remainder
    //
    //   b---arrrrrrrr
    //   [---]--------
    //   [-----]------
    //   [--]---------

    timespec now;
    timespec sleep_time = {0, 0};
    while (true)
    {
        clock_gettime(CLOCK_MONOTONIC, &now);
        sleep_time.tv_nsec = ONE_SEC_IN_NANO / FPS;

        read(fd, pixelBuffer, bufferLen);
        m->driveMatrix2();
        sleep_remainder(now, sleep_time);
    }
}

int main()
{
#ifndef DEBUG
    pid_t pid = fork();
    if (pid == -1)
    {
        return -1;
    }
    if (pid != 0) // if is parent
        exit(EXIT_SUCCESS);

    printf("Starting !\n");

    if (setsid() == -1)
        return -1;
#endif
    if (chdir("/") == -1)
    {
        return -1;
    };

    printf("Getting logdf\n");
    // open file for logging
    logfd = open(DAEMON_LOG_PATH, O_RDWR | O_CREAT);
    if (logfd == -1)
    {
        strerror(errno);
        return -1;
    }

    printf("Got logfd\n");

    // close fds
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // redirect stdin
    // redirect stdout and stderr to the logger
    open("/dev/null", O_RDONLY); // stdin
    dup(logfd);                  // stdout
    dup(logfd);                  // stderr

    printf("Starting Led Daemon:\n############################\n\n");

    printf("PID is %d\n", getpid());

    // try to increase priority
    int currNice = nice(0);
    int newNice = nice(-20);

    if (newNice == currNice)
    {
        printf("Led daemon failed to update priority. Running with lower performance...\n");
    }

    // to get address and access buffer
    //   -open a driver
    //   -get matrix dimensions from IOCTL
    //   -mmap /dev/mem , len=dimensions

    DriverWrapperFactory f;
    DriverWrapper *d = f.CreateDriverWrapper();

    dimensions_t dim;
    dim.row_n = 0xFFFFFFFE; // almost maximum, will be cropped
    dim.col_n = 0xFFFFFFFE;

    if (d->resize(dim.row_n, dim.col_n) == -1)
    {
        printf("Failed to resize\n");
        close(logfd);
        delete (d);
        return -1;
    }

    dim = d->getDimensions();
    if (dim.col_n == (uint32_t)0xFFFFFFFF || dim.row_n == (uint32_t)0xFFFFFFFF)
    {
        printf("Failed to get dimensions\n");
        close(logfd);
        delete (d);
        return -1;
    }

    bufferLen = dim.row_n * dim.col_n * sizeof(uint32_t);

    uint32_t buf[bufferLen / sizeof(uint32_t)];
    pixelBuffer = buf;

    for (uint i = 0; i < bufferLen / sizeof(uint32_t); i++)
    {
        pixelBuffer[i] = 0;
    }

    printf("Got dimensions: %dx%d\n", dim.row_n, dim.col_n);

    printf("Mapped pixelBuffer: %p\n", pixelBuffer);

    // Pixel matrix stuff
    m = new MatrixDriver();
    if (m->Init(pixelBuffer, dim.row_n, dim.col_n))
        printf("Successfully initialized matrix\n\n");
    else
    {
        printf("Failed to initialize matrix\n");
        return -1;
    }

    write(d->getFd(), (char *)pixelBuffer, bufferLen);
    printf("Wrote %ld bytes, clearing the buffer. Expected %d\n", write(d->getFd(), (char *)pixelBuffer, bufferLen), bufferLen);

    // activate signals
    sigset(SIGTERM, sigterm_handler);

    printf("Running!\n");

    run(d->getFd());
}