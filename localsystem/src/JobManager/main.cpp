#include <signal.h>
#include "./JobManager.hpp"

JobManager *jm = nullptr;
void termHandler(int signo)
{
    switch (signo)
    {
    case SIGTERM:
        if (jm != nullptr)
            delete (jm);
        break;

    default:
        break;
    }
}

int main(int argc, char **arg)
{
    signal(SIGTERM, termHandler);
    jm = new JobManager();
    // arg[0] is file path
    // arg[1] is row_n
    // arg[2] is col_n
    // arg[3] is schedulePath

    char *schedulePath = NULL;
    dimensions_t dim;

    switch (argc)
    {
    default:
    case 4:
        schedulePath = arg[3];
        __attribute__((fallthrough));
    case 3:
        dim.col_n = strtol(arg[2], NULL, 10);
        dim.row_n = strtol(arg[1], NULL, 10);
        break;
    case 2: // default args for all
    case 1:
        break;
    }

    bool initRet = false;
    switch (argc)
    {
    default:
    case 4:
        initRet = jm->init(arg, dim, schedulePath);
        break;
    case 3:
        initRet = jm->init(arg, dim);
        break;
    case 2:
    case 1:
        initRet = jm->init(arg);
        break;
    }

    if (!initRet)
    {
        printf("Failed to initialize the job manager, returning...\n");
        return -1;
    }

    printf("Initialized JobManager\n");

    if (jm->parseSchedule() != 0)
    {
        printf("Invalid schedule, returning...");
        return -1;
    }
    jm->spawnDisplayThread();
    jm->spawnScheduleManager();
    jm->jobManager();
}