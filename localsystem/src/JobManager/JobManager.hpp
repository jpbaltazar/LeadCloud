#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/stat.h>

#include <thread>
#include <mutex>
#include <fstream>
#include <iostream>

#include <json/json.h>

#include "../Driver/DriverWrapper.hpp"

//gets a list of schedule and starts processes

struct JobInstance_t{
    //general
    char **arg; //first arg is the path

    //scheduling
    unsigned int duration; //in secs

    int id; //for retroactive referencing of jobs
    pid_t pid;

    bool toRemove;
};

enum ScheduleType{
    invalid,
    daily,
    weekly,
    monthly,
    yearly
};

enum ScheduleEventType{
    invalidType,
    //header,
    setTime,
    addJob,
    removeJob
};

struct ScheduleEntry_t{
    uint32_t weekTime; //week time and hour (clipped by 7 days)
    
    bool isAdd;

    std::shared_ptr<JobInstance_t> job;

    ScheduleEntry_t* next;

};

struct JobRunningInstance_t{ //LL list
    int jobTableIndex;

    JobRunningInstance_t* next;
};

class JobManager
{
private:
    DriverWrapper *dw = nullptr;
    
    static int id;

    int fbfd = -1;
    uint fbsize = 0;

    dimensions_t dim;

    char fbpath[256];
    uint32_t* framebuffer = NULL;

    const char* schedulePath = JOBMANAGER_SCHEDULE_PATH;
    ScheduleEntry_t* scheduleEntryHead = NULL;
    ScheduleType scheduleType = invalid; 

    std::shared_ptr<JobInstance_t> jobTable[JOBMANAGER_JOB_TABLE_SIZE] = {0};

    JobRunningInstance_t* runningJobsHead = NULL;
    JobRunningInstance_t * currRunningJob = NULL;
    std::shared_ptr<JobInstance_t> baseJob = NULL;

    bool initJob(std::shared_ptr<JobInstance_t> job);
    bool addToRunningJobs(std::shared_ptr<JobInstance_t> job);


    int currJobTableIndex = -1; //for number assignment
    
    int getJobTableIndex();

    int scheduleManager();//requires init and a current schedule
    uint getClippedTime();

    void checkRunningJobs();
    std::shared_ptr<JobInstance_t> getNextJob();

    static void usr1Handler(int signum);
    static void usr1SigAction(int signo, siginfo_t *info, void *context);

    void clean(); //for cleanup

    std::thread* scheduleManagerThread;
    std::thread* jobManagerThread;
    std::thread* displayThread;
    

    std::mutex runningJobMutex;
public:
    JobManager();
    ~JobManager();

    bool init(char** originalProcessArgs, dimensions_t dim = (dimensions_t){32, 64}, char* schedulePath = JOBMANAGER_SCHEDULE_PATH);
    int parseSchedule();
    void spawnScheduleManager();
    void spawnJobManager();
    void spawnDisplayThread();

    int jobManager();

};

//The job manager will initialize with the current dimensions
//It will then parse a schedule
//It will then start a ScheduleManagerThread to go throught the schedule and get jobs
//It will then start a 