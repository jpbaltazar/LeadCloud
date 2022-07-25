#ifndef LED_DEFS_H
#define LED_DEFS_H

#ifndef uint32_t
#define uint32_t unsigned int
#define uint64_t unsigned long
#endif

#define DAEMON_SETTINGS_PATH "root/.config/ledDaemonSettings"
#define DAEMON_LOG_PATH "/var/log/ledDaemon.log"

#define LED_DD_NAME "ledmatrix"
#define LED_DD_FIRST_NAME "ledmatrix0"
#define LED_DD_FIRST_PATH "/dev/ledmatrix0"
#define LED_DD_NAME_FMT "ledmatrix%d"
#define LED_DD_PATH_FMT "/dev/ledmatrix%d"

#define LED_DEFAULT_ROW_N 32
#define LED_DEFAULT_COL_N 64

#define LED_DD_COUNT 4

#define LED_DD_MAJOR 0
#define LED_DD_MINOR 0

////////////////////
//IOCTL
////////////////////
#define LED_IOC_MAGIC 'z'

//pairs of integers mapped to the same size
#define LED_IORPOS _IOR(LED_IOC_MAGIC, 0, uint64_t *) // (x << 32 | y) //needs uint64_t pointer
#define LED_IOWPOS _IOW(LED_IOC_MAGIC, 1, uint64_t) // (x << 32 | y) //needs uint64_t value
#define LED_IORDIM _IOR(LED_IOC_MAGIC, 2, uint64_t *) // (w << 32 | y) //needs uint64_t pointer
#define LED_IOWDIM _IOW(LED_IOC_MAGIC, 3, uint64_t) // (w << 32 | y) //needs uint64_t value

#define LED_IORGDIM _IOR(LED_IOC_MAGIC, 4, uint64_t *) //global dimensions

//get physical address
//#define LED_IORPHYS _IOR(LED_IOC_MAGIC, 5, off_t *) //needs phys_addr_t pointer

////////////////////

struct position_t{
    uint32_t x;
    uint32_t y;
};
typedef struct position_t position_t;

struct dimensions_t{
    uint32_t row_n;
    uint32_t col_n;
};
typedef struct dimensions_t dimensions_t;

union to_ioctl{
    dimensions_t dim;
    position_t pos;

    uint64_t uns64;
};

typedef union to_ioctl to_ioctl;

struct Settings_t{
    position_t pos;
    int fps;
};

//////////////////////JOBMANAGER//////////////////////////////////

#define JOBMANAGER_SCHEDULE_PATH (char *)"~/schedule"
#define JOBMANAGER_FRAMEBUFFER_PATH_FMT "/tmp/jobmanagerfb%d"

#define JOBMANAGER_JOB_TABLE_SIZE 50

#endif //LED_DEFS_H
