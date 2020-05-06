#include <iostream>
#include <cstring>
#include <csignal>
#include <cstdint>
#include <cstdio>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include "DevicesOps.h"

static sem_t g_down_sem;

using namespace std;

DevicesOps::DevicesOps(/* args */)
{
    int ret = 0;
    private_fd = open("/dev/fpsdev0", O_RDWR);
    if (private_fd < 0)
    {
        throw "open fpsdev0 fail";
    }

    ioctl(private_fd, CDFINGER_CHANGER_CLK_FREQUENCY, 9600000);
    ioctl(private_fd, CDFINGER_INIT_IRQ);
    ioctl(private_fd, CDFINGER_POWER_ON);

    cfpHandlerRegister(sig_handler);
    ret = sem_init(&g_down_sem, 0, 0);
    if(ret < 0)
    {
        throw "sem init failed";
    }

}

DevicesOps::~DevicesOps()
{
}

void DevicesOps::sig_handler(int signo)
{
    if (signo == SIGIO)
    {
        cout<<"Interrupt signal!!!"<<endl;
        sem_post(&g_down_sem);
    }
    if (signo == SIGINT)
    {
        cerr<<"receive interrupt signal error"<<endl;
        exit(0);
    }
}

int DevicesOps::cfpHandlerRegister(void (*func)(int))
{
    struct sigaction action;
    if (func == NULL)
    {
        cerr<<"Func to register is NULL"<<endl;
        return -1;
    }
    memset(&action, 0, sizeof(action));
    action.sa_handler = func;
    action.sa_flags = 0;
    sigaction(SIGIO, &action, NULL);
    sigaction(SIGINT, &action, NULL);
    fcntl(private_fd, F_SETOWN, getpid());
    fcntl(private_fd, F_SETFL, fcntl(private_fd, F_GETFL) | FASYNC);
    return 0;
}

int DevicesOps::getDeviceHandle() const
{
    return private_fd;
}

void DevicesOps::setDeviceHandle(int fd)
{
    private_fd = fd;
}

int64_t DevicesOps::getTimeMsec()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int DevicesOps::func_sem_timedwait(int time)
{
    int ret = 0;
    struct timespec timewait;
    timewait.tv_sec = getTimeMsec() / 1000 + time;
    timewait.tv_nsec = 0;

    ret = sem_timedwait(&g_down_sem, &timewait);
   
    return ret;
}

int DevicesOps::WaitSemCall(int time)
{
    int ret = -1;

    while ((ret = func_sem_timedwait(time)) == -1 && errno == EINTR)
        continue; /* Restart if interrupted by handler */

    if (ret == -1)
    {
        if (errno == ETIMEDOUT)
            cerr<<"********timeout********"<<endl;
    }
    return ret;
}


