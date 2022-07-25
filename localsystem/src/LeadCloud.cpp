#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <resolv.h>
#include <pthread.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include <iostream>
#include <fstream>

#include <json/json.h>

#include "../lib/Drawer.hpp"
#include "../Driver/DriverWrapper.hpp"

#define REQUESTBUFFERSIZE 4096
#define CONFIGFILEPATH "./leadcloud.config"

#define SCHEDULEPATH_JM (char *)"./schedule.json"
#define SCHEDULEPATH_LOCAL (char *)"./VisualJob/schedule.json"

#define TCPPORT 38888

pid_t JobManagerPid = -1;
bool hasConfig = true;
bool initialized = false;

DriverWrapperFactory f;
DriverWrapper *d = f.CreateDriverWrapper();
uint32_t screenshotBuf[32 * 64];

enum RequestType
{
    statusUpdate = 0,
    sendScheduleMetadata,
    newSchedule,
    shutdownType, // shutdown is already defined as a function
    newConfig,
    closeConnection,
    screenshot,
};

void requestHandler(int sd)
{
    char buffer[REQUESTBUFFERSIZE];

    printf("handling requests...\n");

    ssize_t read;
    while (true)
    {
        // printf("Receiving...\n");
        read = recv(sd, buffer, REQUESTBUFFERSIZE, 0);
        if (read == 0)
        {
            continue;
        }
        else if (read < 0)
        {
            break;
        }

        printf("Got request!\n");

        Json::Value root;
        Json::CharReaderBuilder builder;
        JSONCPP_STRING err;

        const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        if (!reader->parse(buffer, buffer + read, &root, &err))
        {
            return;
        }

        RequestType requestType;

        switch (root["RequestType"].asInt())
        {
        case statusUpdate:
            requestType = statusUpdate;
            break;
        case sendScheduleMetadata:
            requestType = sendScheduleMetadata;
            break;
        case newSchedule:
            requestType = newSchedule;
            break;
        case shutdownType:
            requestType = shutdownType;
            break;
        case newConfig:
            requestType = newConfig;
            break;
        case closeConnection:
            requestType = closeConnection;
            break;
        case screenshot:
            requestType = screenshot;
        default:
            break;
        }

        printf("scheduleType = %d\n", requestType);

        if (requestType == statusUpdate)
        {
            const char *reply = "online";
            printf("Replying: %s\n", reply);

            send(sd, reply, strlen(reply), 0);
        }
        else if (requestType == sendScheduleMetadata)
        {
            const char *reply = "not implemented";
            printf("Replying: %s\n", reply);

            send(sd, reply, strlen(reply), 0);
        }
        else if (requestType == newSchedule)
        {
            printf("%s\n", root["JsonSchedule"].asCString());

            const char *reply = "OK";
            printf("Replying: %s\n", reply);

            send(sd, reply, strlen(reply), 0);

            std::ofstream scheduleFile(SCHEDULEPATH_LOCAL, std::ofstream::out);
            scheduleFile << root["JsonSchedule"].asCString();
            scheduleFile.close();

            if (JobManagerPid > 0)
            {
                kill(JobManagerPid, SIGUSR1);
            }
        }
        else if (requestType == shutdownType)
        {
            const char *reply = "not implemented";
            printf("Replying: %s\n", reply);

            send(sd, reply, strlen(reply), 0);

            if (JobManagerPid > 0)
            {
                kill(JobManagerPid, SIGTERM);
            }
        }
        else if (requestType == newConfig)
        {
            const char *reply = "OK";
            printf("Replying: %s\n", reply);

            std::ofstream configFile(CONFIGFILEPATH, std::ofstream::out);
            configFile << root["Config"].asCString();
            configFile.close();

            printf("Config: %s\n", root["Config"].asCString());

            hasConfig = true;

            send(sd, reply, strlen(reply), 0);
        }
        else if (requestType == closeConnection)
        { // just close it
            break;
        }
        else if (requestType == screenshot)
        {
            DriverWrapper *d = f.CreateDriverWrapper();
            d->resize(32, 64);

            write(d->getFd(), screenshotBuf, 32 * 64 * sizeof(uint32_t));

            send(sd, screenshotBuf, sizeof(screenshotBuf), 0);
        }
    }
}

int setupTCPServer(sockaddr_in *address, int *listen_sd)
{
    memset(address, 0, sizeof(sockaddr_in));
    *address = {
        AF_INET,
        htons(TCPPORT),
        INADDR_ANY};

    *listen_sd = socket(PF_INET, SOCK_STREAM, 0);
    if (*listen_sd < 0)
    {
        return *listen_sd;
    }

    int error;
    if ((error = bind(*listen_sd, (sockaddr *)address, sizeof(sockaddr_in))) != 0)
    {
        return error;
    }

    if ((error = listen(*listen_sd, 10)) != 0)
    { // only one should connect at a time
        return error;
    }
    return 0;
}

void launchJobManager()
{
    printf("Launching Job Manager!\n");

    JobManagerPid = fork();
    if (JobManagerPid == 0)
    { // child
        char *argv[5];

        printf("Chdir returned %d\n", chdir("./VisualJob/"));

        argv[0] = (char *)"./JobManager";
        argv[1] = (char *)"32";
        argv[2] = (char *)"64";
        argv[3] = SCHEDULEPATH_JM;
        argv[4] = NULL;

        printf("Exec returned %d\n", execv(argv[0], argv));
    }

    printf("Forked with pid %d\n", JobManagerPid);
}

int serveRequests()
{
    sockaddr_in address;
    int listen_sd;

    if (hasConfig)
    {
        launchJobManager();
        initialized = true;
    }

    int error;
    if ((error = setupTCPServer(&address, &listen_sd)) != 0)
    {
        return error;
    }
    else
    {

        while (true)
        {

            socklen_t sl = sizeof(address);
            int sd = accept(listen_sd, (sockaddr *)&address, &sl);
            printf("\nAccepted connection!\n");
            if (sd != -1)
            {
                requestHandler(sd);
            }
            else
            {
                printf("Failed to connect with sd %d\n", sd);
            }

            // launch the job manager if it the system obtained the configuration
            if (!initialized)
            {
                if (hasConfig)
                {
                    printf("Got config, starting JobManager\n");

                    launchJobManager();
                    initialized = true;
                }
            }
        }
    }

    return 0;
}

#define LEADCLOUD_WEBSERVER_URI "http://10.42.0.1:8080/DeviceSelfRegister"

int showConfigKey(char *key)
{
    long row_n = 32;
    long col_n = 64; // minimum sizes

    uint32_t pixelBuffer[32 * 64];

    DriverWrapperFactory f;
    DriverWrapper *d = f.CreateDriverWrapper();
    if (d == nullptr)
    {
        return -EBUSY; // no available device driver instance
    }

    d->resize(32, 64);
    int driverFd = d->getFd();

    Drawer drawer(pixelBuffer, row_n, col_n);

    drawer.fill(0);
    ../
        drawer.drawTextSmall(row_n / 4 - ASCII_HEIGHT / 2, 0, "Config:", 0xFF0000FF, 0);
    drawer.drawTextSmall(3 * row_n / 4 - ASCII_HEIGHT / 2, ASCII_WIDTH / 2, key, 0xFFFFFFFF, 0);

    write(driverFd, pixelBuffer, sizeof(uint32_t) * row_n * col_n);

    return 0; // no need to keep driving it
}

const char *getConfigKey()
{
    std::string uri = "wget -O - ";
    uri += LEADCLOUD_WEBSERVER_URI; //"wget http://10.42.0.1:8080/DeviceSelfRegister"
    // uri += " > /dev/null";

    std::array<char, 128> buffer;
    std::string result;

    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(uri.c_str(), "r"), pclose);
    if (!pipe)
    {
        printf("popen() failed!");
        return nullptr;
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }

    return result.c_str();
}

int checkForConfigs()
{
    struct stat buf;
    if (stat(CONFIGFILEPATH, &buf) == 0)
    {
        return 1; // it exists
    }

    return 0; // does not exist
}

int main()
{

    if (checkForConfigs() == 0)
    {
        printf("Couldn't find leadcloud.config file, requesting new configuration...\n");

        const char *key = getConfigKey();
        // exists for the first read
        char keyBuf[16];

        strcpy(keyBuf, key);

        // display the key in here
        printf("Key: %s\n", keyBuf);

        showConfigKey(keyBuf);

        hasConfig = false;
    }

    return serveRequests();
}
