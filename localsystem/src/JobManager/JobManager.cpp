#include "JobManager.hpp"

static JobManager* jobManager_p = NULL;
static char* exePath = NULL;
static char** args = NULL;

JobManager::JobManager() //framebuffer location
{

}

JobManager::~JobManager()
{
    clean();
}

void JobManager::clean(){

    //account for:
    //Schedule entries
    //Job instances in schedule entries
    //Job table entries
    //JobRunningInstances
    //processes started
    //driver wrapper

    JobRunningInstance_t** currJRI;
    ScheduleEntry_t** currSE;

    for(int i = 0; jobTable[i] != nullptr && i < JOBMANAGER_JOB_TABLE_SIZE; i++){
        kill(jobTable[i]->pid, SIGTERM);
        kill(jobTable[i]->pid, SIGCONT); //so it can actually finish
        printf("\tSIGTERM -> pid %d\n", jobTable[i]->pid);
        jobTable[i].reset(); //dereference them
    }

    //JobRunningInstances
    currJRI = &runningJobsHead;
    while (*currJRI == NULL)
    {
        JobRunningInstance_t* toRemove = *currJRI;
        (*currJRI) = toRemove->next;

        free(toRemove);
    }

    //Schedule entries
    currSE = &scheduleEntryHead;
    while (*currSE == NULL)
    {
        ScheduleEntry_t* toRemove = *currSE;
        (*currSE) = toRemove->next;
        toRemove->job.reset(); //deletes all references to job, removing the job
        free(toRemove);
    }

    delete(dw);

    printf("////////////////////////////////////\n");
    printf("Destroying JobManager...\n");
    printf("////////////////////////////////////\n");
    
    remove(fbpath);

    printf("fbpath is not the problem");
}

bool JobManager::init(char** originalProcessArgs, dimensions_t dim, char* schedulePath){
    //initialize parameters
    if(originalProcessArgs == NULL){
        printf("No args passed, returning...\n");
        return -1;
    }

    exePath = originalProcessArgs[0];
    args = originalProcessArgs;
    jobManager_p = this;

    if(exePath == NULL || args == NULL || args[0] == NULL){
        printf("No valid args passed, returning...\n");
        return -1;
    }
    
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGUSR1);

    
    signal(SIGUSR1, JobManager::usr1Handler);
    //if this is after the SIGUSR1,
    //the signal mask is inherited from the signal handler
    // which will have SIGUSR1 blocked, we need to unblock it
    sigprocmask(SIG_UNBLOCK, &sigset, NULL);

    //dimensions
    if(dim.row_n <= 0 || dim.col_n <= 0)
        return false;

    this->dim = dim; //save dimensions

    this->schedulePath = schedulePath;

    //create a framebuffer
    bool fbfdFound = false;
    for(int i = 0; i < 100; i++){ //obvsly less than 100 job managers
        sprintf(fbpath, JOBMANAGER_FRAMEBUFFER_PATH_FMT, i);
        struct stat buf;
        if(stat(fbpath, &buf)){
            if((fbfd = open(fbpath, O_CREAT | O_RDWR | O_CLOEXEC))){
                fbfdFound = true;
                break;
            }
        }
    }

    if(!fbfdFound)
        return false;

    fbfd = open(fbpath, O_RDWR | O_CREAT | O_CLOEXEC); //it was verified to not exist so no need to verify it
    fbsize = dim.col_n * dim.row_n * sizeof(uint32_t);
    fallocate(fbfd, 0, 0, fbsize); //has to be allocated to be used

    framebuffer = (uint32_t*)mmap(NULL, fbsize, PROT_WRITE | PROT_READ, MAP_SHARED, fbfd, 0);
    if(framebuffer == MAP_FAILED)
        return false;

    //get driver wrapper
    DriverWrapperFactory f;

    dw = f.CreateDriverWrapper();
    if(dw == nullptr){
        munmap(framebuffer, fbsize);
        framebuffer = NULL;
        printf("Could not create driver wrapper, exiting...\n");
        return false;
    }
    if(dw->resize(dim.row_n, dim.col_n)){
        munmap(framebuffer, fbsize);
        framebuffer = NULL;
        close(dw->getFd());
        return false;
    }

    for(uint i = 0; i < fbsize/sizeof(uint32_t); i++){ //clear it
        framebuffer[i] = 0;
    }

    write(dw->getFd(), framebuffer, fbsize); //reset it

    baseJob = std::make_shared<JobInstance_t>();

    //initialization of the base job as a fallback for an empty schedule
    char ** baseJobArgs = (char**) malloc(sizeof(char*) * 5); //file fbpath 32 64

    baseJobArgs[0] = (char *)malloc(256 * sizeof(char));
    sprintf(baseJobArgs[0], "baseJob");
    baseJobArgs[1] = (char *)malloc(256 * sizeof(char));
    sprintf(baseJobArgs[1], fbpath);
    baseJobArgs[2] = (char *)malloc(50 * sizeof(char));
    sprintf(baseJobArgs[2], "%d", dim.row_n);
    baseJobArgs[3] = (char *)malloc(50 * sizeof(char));
    sprintf(baseJobArgs[3], "%d", dim.col_n);
    baseJobArgs[4] = NULL;

    *baseJob = {
        .arg = baseJobArgs,
        .duration = 1,
        .id = 0,
        .pid = -1,
        .toRemove = false
    };

    jobTable[getJobTableIndex()] = baseJob;
    initJob(baseJob); //forks and starts the job but immediatly stops it

    return true;
}

/*
ssize_t readLine(int fd, char* buf, ssize_t size){
    if(buf == nullptr || size < 0)
        return -1;
    
    ssize_t readN = read(fd, buf, size);
    if(readN < 1)
        return readN; //-1 and 0

    char *nl = strchr(buf, '\n');

    if(nl == buf + size - 1){ //didn't find a new line
        return -1;
    }

    off_t off = nl - buf;
    lseek(fd, off - readN + 1, SEEK_CUR); //return it there

    *nl = '\0';
    return (nl - buf);
}
*/

int JobManager::parseSchedule(){
    if(strlen(schedulePath) == 0){ //no path given, no jobs to be run
        return 0;
    }
    
    struct stat buf;
    if(stat(schedulePath, &buf) != 0){
        //if the file does not exist, just return
        return 0;
    }

    Json::Value root;
    std::ifstream ifs;

    ifs.open(schedulePath);

    Json::CharReaderBuilder builder;
    JSONCPP_STRING errs;

    uint weektime = 1;
    ScheduleEntry_t** curr = &scheduleEntryHead;

    if(!Json::parseFromStream(builder, ifs, &root, &errs)){
        std::cout << errs << std::endl;
        return -1;
    }

    scheduleType = (ScheduleType)root["header"]["scheduleType"].asInt();

    auto entries = root["entries"];
    
    uint entryTime;
    int argc;

    for(uint i = 0; i < entries.size(); i++){
        Json::Value entry = entries[i];
        switch (entry["entryType"].asUInt())
        {
        case invalidType:
            printf("Entry type was empty!\n");
            break;
            
        case setTime: //set time
            entryTime = entry["time"].asUInt();
            if(entryTime == 0){
                printf("Invalid time / time was zero, assuming 1 (immediate)\n");
                entryTime = 1;
            }
            //printf("Set time to %d\n", entryTime);
            weektime = entryTime;
            break;
        case addJob: //add task

            *curr = new ScheduleEntry_t();
            if(*curr == nullptr)
                this->~JobManager();

            (*curr)->job = std::make_shared<JobInstance_t>();

            (*curr)->job->id = entry["id"].asUInt();
            if((*curr)->job->id == 0){
                printf("Job id 0 is forbidden\n");
                return -1;
            }
            
            (*curr)->job->duration = entry["duration"].asUInt();

            argc = entry["args"].size();
            if(argc == 0){
                printf("No args for job, can't progress\n");
                return -1;
            }

            //+ 4
            //  framebuffer path
            //  row_n
            //  col_n
            //  terminating NULL
            (*curr)->job->arg = (char **)malloc(sizeof(char*) * (argc + 4));
            if((*curr)->job->arg == NULL)
                return -1;
            
            for(int j = 0; j < argc + 4; j++){
                (*curr)->job->arg[j] = NULL;
            }

            (*curr)->job->arg[0] = (char *)malloc(256 * sizeof(char)); //256 per arg
            if(curr == NULL)
                return -1;
            sprintf((*curr)->job->arg[0], "%s", entry["args"][0].asCString());

            (*curr)->job->arg[1] = (char *)malloc(256 * sizeof(char)); 
            if(curr == NULL)
                return -1;
            sprintf((*curr)->job->arg[1], "%s", fbpath);

            (*curr)->job->arg[2] = (char *)malloc(50 * sizeof(char)); 
            if(curr == NULL)
                return -1;
            sprintf((*curr)->job->arg[2], "%d", dim.row_n);

            (*curr)->job->arg[3] = (char *)malloc(50 * sizeof(char)); 
            if(curr == NULL)
                return -1;
            sprintf((*curr)->job->arg[3], "%d", dim.col_n);

            for(int j = 4; j < argc + 3; j++){
                (*curr)->job->arg[j] = (char *)malloc(256 * sizeof(char));
                if(curr == NULL)
                    return -1;
                
                sprintf((*curr)->job->arg[j], "%s", entry["args"][j - 3].asCString());
            }

            (*curr)->weekTime = weektime;
            (*curr)->isAdd = true;
            (*curr)->job->toRemove = false;

            curr = &((*curr)->next);

            break;
        case removeJob: //remove task

            *curr = new ScheduleEntry_t();

            (*curr)->job = std::make_shared<JobInstance_t>();

            (*curr)->job->id = entry["id"].asUInt();
            if((*curr)->job->id == 0){
                printf("Job id 0 is forbidden\n");
                return -1;
            }

            (*curr)->weekTime = weektime;
            (*curr)->isAdd = false;

            curr = &((*curr)->next);
            break;
        default: //error
            printf("Unknown entry type!\n");
            break;
        }
    }

    if(scheduleType == invalid){
        printf("Invalid schedule, no header? (entry type 1 and \"scheduleInterval\" either daily(1), weekly(2), monthly(3) or yearly(4))\n");
        clean();
        return -1;
    }

    printf("Successfully read schedule.\n");
    ifs.close();
    return 0;
}

int JobManager::getJobTableIndex(){
    currJobTableIndex++; //will usually be set to the last one, this avoids checking one that will be filled
    for(int i = 0; i < JOBMANAGER_JOB_TABLE_SIZE; i++){
        int effIndex = (i + currJobTableIndex) % JOBMANAGER_JOB_TABLE_SIZE;
        if(jobTable[effIndex] == NULL){
            currJobTableIndex = effIndex;
            return effIndex;
        }
    }

    return -1;
}

bool JobManager::initJob(std::shared_ptr<JobInstance_t> job){
    pid_t forked = fork();
    if(forked < 0)
        return false;
    else if(forked == 0){//child
        int ret = execv(job->arg[0], job->arg);
        /*if(setpgid(0, 0)){
            printf("Couldn't set gpid\")
            return -1;
        }*/
        printf("Execv returned %d\n", ret);
        if(ret == -1){
            printf("Errno was %d\n", errno);
        }
        return false; //it will not happen, only if execv returns -1 from an error
    }
    else{ //parent
        job->pid = forked;
        
        kill(forked, SIGSTOP); //start the jobs and immediately stop them 
        /*if(currRunningJob == NULL){ //select the head
            currRunningJob = (JobRunningInstance_t **)&runningJobsHead;
        }*/
    }

    return true;
}

bool JobManager::addToRunningJobs(std::shared_ptr<JobInstance_t> job){
    int index = getJobTableIndex();
    jobTable[index] = job; 
    printf("Started job \"%s\", index %d with pid %d\n", job->arg[0], job->id, job->pid);
    if(index == -1){
        //TODO solve this exit point
        //delete everything, kill all jobs, not having anymore available jobs is not a good idea
        return false;
    }


    JobRunningInstance_t** currJob = (JobRunningInstance_t **)&runningJobsHead;
    while ((*currJob) != NULL)
        currJob = &((*currJob)->next);
    
    
    *currJob = (JobRunningInstance_t *)calloc(1, sizeof(JobRunningInstance_t));
    if(*currJob == NULL)
        return -1;
    (*currJob)->jobTableIndex = index;

    return true;
}

//SCHEDULING

uint JobManager::getClippedTime(){
    time_t rawTime;
    tm *currTime;
    
    time(&rawTime);
    currTime = localtime(&rawTime);
    
    uint clippedTime = 0;
    switch (scheduleType)
    {
    case yearly:
        clippedTime = (((currTime->tm_yday * 24 + currTime->tm_hour) * 60) + currTime->tm_min) * 60 + currTime->tm_sec;
        break;
    case monthly:
        clippedTime = (((currTime->tm_mday * 24 + currTime->tm_hour) * 60) + currTime->tm_min) * 60 + currTime->tm_sec;
        break;
    case weekly:
        clippedTime = (((currTime->tm_wday * 24 + currTime->tm_hour) * 60) + currTime->tm_min) * 60 + currTime->tm_sec;
        break;
    case daily:
        clippedTime = ((currTime->tm_hour * 60) + currTime->tm_min) * 60 + currTime->tm_sec;
        break;
    default:
        return 0;
    }

    return clippedTime; 
}

int JobManager::scheduleManager(){

    while (true)
    {
        ScheduleEntry_t* curr = scheduleEntryHead;
        
        uint currTime = 0;
        while (curr != NULL) //list is over
        {
            
            while ((currTime = getClippedTime()) < curr->weekTime)
            {
                sleep(1);
            }

            printf("New action at: %d\n", currTime);

            if(curr->isAdd){ //adding jobs is harmless
                if(!initJob(curr->job)){
                    return -1;
                }

                if(!addToRunningJobs(curr->job)){
                    return -1;
                }

            } //this time shit is wrong TODO fix
            else{
                for(int i = 0; i < JOBMANAGER_JOB_TABLE_SIZE; i++){
                    if(jobTable[i]->id == curr->job->id){
                        jobTable[i]->toRemove = true;
                        break;
                    }
                }

            }

            curr = curr->next;
        }
        
        while (true) //should only restart if it is the next day/week/month, this is a placeholder TODO
            sleep(1);
    }

    return 0;
}

void JobManager::spawnScheduleManager() {
    if(framebuffer == NULL || scheduleEntryHead == NULL){
        printf("Failed to start Schedule Manager thread, framebuffer was %n, scheduleEntryHead was %p\n", framebuffer, scheduleEntryHead);
        return;
    }

    printf("Started Schedule Manager thread\n");


    scheduleManagerThread = new std::thread( [this] { this->scheduleManager(); });
    scheduleManagerThread->detach();
}

//JOB MANAGING

void JobManager::usr1Handler(int signum){
    //free all allocated space and execve the stack space to the original
    //works because all data gets invalidated, and is therefore deletable


    switch (signum)
    {
    case SIGUSR1:
        jobManager_p->clean(); //simply call destructor and start the process again
        printf("New schedule, restarting...\n");
        //all heap allocations removed
        //finally restart program
        execv(exePath, args);
        
        break;
    default:
        break;
    }
}

void JobManager::checkRunningJobs(){
    bool reloadSchedule = false;

    for(JobRunningInstance_t** curr = &runningJobsHead; *curr != NULL;)
    {
        if(jobTable[(*curr)->jobTableIndex]->toRemove){ //remove the job from running pool of jobs
            jobTable[(*curr)->jobTableIndex]->toRemove = false; //already removed it
            
            JobRunningInstance_t* toRemove = *curr;
            (*curr) = toRemove->next;
            kill(jobTable[toRemove->jobTableIndex]->pid, SIGKILL);
            jobTable[toRemove->jobTableIndex]->pid = -1;
            free(toRemove);

            reloadSchedule = true; //if action was taken, reload the schedule
        }
        else{
            curr = &((*curr)->next); //only if nothing was removed, do you remove that one
        }
    }
    
    if(reloadSchedule){
        currRunningJob = NULL;
    }

}

std::shared_ptr<JobInstance_t> JobManager::getNextJob(){
    checkRunningJobs();

    static bool noJobs = false;

    if(currRunningJob == NULL && runningJobsHead == NULL){
        if(!noJobs){
            printf("No jobs, running base job...\n");
            noJobs = true;
        }
    }
    else if(currRunningJob == NULL){
        printf("(Re)starting new schedule...\n");
        currRunningJob = runningJobsHead;
        noJobs = false;
    }
    else{
        currRunningJob = currRunningJob->next;
        if(currRunningJob == NULL){ //if it is the end, restart it //tbh, it could be looped, but more functionality could be implemented here
            currRunningJob = runningJobsHead;
        }

        noJobs = false;
    }

    if(currRunningJob != NULL)
        return jobTable[currRunningJob->jobTableIndex]; //TODO fix currRunning job being NULL
    else 
        return baseJob;
}

int JobManager::jobManager(){
    while (true)
    {
        std::shared_ptr<JobInstance_t> runningJob = getNextJob();
        
        kill(runningJob->pid, SIGCONT);
        sleep(runningJob->duration);
        kill(runningJob->pid, SIGSTOP);
    }

    return 0;
}

void JobManager::spawnJobManager() {
    if(framebuffer == NULL){
        printf("Failed to launch Job Manager thread, framebuffer was %n\n", framebuffer);
        return;
    }
    printf("Started Job Manager thread\n");

    jobManagerThread = new std::thread( [this] { this->jobManager(); } );
}

void JobManager::spawnDisplayThread(){ //probably harmless
    if(dw == NULL){
        printf("Driver wrapper was NULL!\n");
        return;
    }
    if(framebuffer == NULL || dw->getFd() <= 0){
        printf("Failed to launch Display thread, framebuffer was %n, and fbfd was %d\n", framebuffer, dw->getFd());
        return;
    }
    printf("Started display thread\n");
    displayThread = new std::thread( [this] {
        while (true)
        {
            timespec sleepAmount = {0, (long)(1e9/30) - 1000};
            nanosleep(&sleepAmount, NULL);
            write(dw->getFd(), framebuffer, fbsize);
        }
    });
}