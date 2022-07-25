#ifndef DRIVER_WRAPPER_H
#define DRIVER_WRAPPER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdint.h>

#include "../lib/ledDefs.h"

/*
    Provide an interface for the driver, avoid complicated ioctl calls and open/close/ones

    Factory creates a working DriverWrapper or returns NULL

    Use DriverWrapper functions to perform configuration of the device driver

    Use the file descriptor for the reads/writes
*/

class DriverWrapper
{
private:
    int driverFd = -1;
    position_t pos;
    dimensions_t dim;

public:
    DriverWrapper(/* args */);
    ~DriverWrapper();

    int getFd();
    int resize(uint32_t row_n, uint32_t col_n); //-1 if failed / 0 if succeeded/ 1 if trimmed size
    void reposition(uint32_t x, uint32_t y);

    position_t getPos();
    dimensions_t getDimensions();

    // off_t getPhysicalAddress();

    friend class DriverWrapperFactory;
};

// FACTORY

class DriverWrapperFactory
{
public:
    DriverWrapperFactory();
    ~DriverWrapperFactory();
    DriverWrapper *CreateDriverWrapper();
};

#endif // DRIVER_WRAPPER_H