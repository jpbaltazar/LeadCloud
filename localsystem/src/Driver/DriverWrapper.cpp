#include "DriverWrapper.hpp"

DriverWrapper::DriverWrapper(){

}

DriverWrapper::~DriverWrapper(){
    if(driverFd != -1){
        close(driverFd);
    }
}

int DriverWrapper::getFd(){
    return driverFd;
}

position_t DriverWrapper::getPos(){
    position_t tmp = {(uint32_t)-1, (uint32_t)-1}; //bonkers initialization
    if(ioctl(driverFd, LED_IORPOS, &tmp) == 0){
        pos = tmp;
    }
    
    return tmp;
}

dimensions_t DriverWrapper::getDimensions(){
    dimensions_t tmp = {(uint32_t)-1, (uint32_t)-1};
    if(ioctl(driverFd, LED_IORDIM, &tmp) == 0){
        dim = tmp;
    }

    return tmp;
}

int DriverWrapper::resize(uint32_t row_n, uint32_t col_n){
    dimensions_t d = {row_n, col_n};
    int ret = ioctl(driverFd, LED_IOWDIM, d);
    if(ret == -1){ //complete error
        printf("Resize ioctl call failed\n");
    }

    return ret;
}

void DriverWrapper::reposition(uint32_t x, uint32_t y){
    position_t p = {x, y};
    ioctl(driverFd, LED_IOWPOS, p); //cannot fail
}

/*off_t DriverWrapper::getPhysicalAddress(){
    off_t addr;
    ioctl(driverFd, LED_IORPHYS, &addr);
    
    return addr;
}*/

//FACTORY

DriverWrapperFactory::DriverWrapperFactory(){

}

DriverWrapperFactory::~DriverWrapperFactory(){

}

DriverWrapper* DriverWrapperFactory::CreateDriverWrapper(){
    DriverWrapper *d;

    d = new DriverWrapper();
    if(d == NULL)
        return NULL;

    char led_dd_path[50];

    int i;
    for(i = 0; i < LED_DD_COUNT; i++){
        // /dev/ledmatrix[i]
        sprintf(led_dd_path, LED_DD_PATH_FMT, i);
                
        d->driverFd = open(led_dd_path, O_RDWR | O_SYNC | O_CLOEXEC);
        if(d->driverFd > 0){ 
            d->resize(0, 0);
            d->reposition(0, 0);
            d->pos = {0, 0};
            d->dim = {0, 0};
            
            return d;
        }
    }
    printf("No ledmatrix drivers available (searched up to %d)\n", i);
    free(d);
    return NULL;
}
