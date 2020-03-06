#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sched.h>
#include <time.h>

#include "sensor_ctl.h"

int fd = 0;

unsigned long long cfp_get_uptime()
{
    struct timeval cur;
    long long int timems = 0;
    gettimeofday(&cur, NULL);
    timems = (long long int)(cur.tv_sec * 1000 + (cur.tv_usec) / 1000);
    return timems;
}

int main(int argc, char *argv[])
{
    int ret = 0;
    unsigned long long t0,t1;
    int i = 0;
    int j = 0;
    float x = 24.7;
    float y = 36.8;

    fd = open("/dev/fpsdev0", O_RDWR);
    if (fd < 0)
    {
        printf("open fpsdev0 fail");
        return -1;
    }

    // ret = ioctl(fd, CDFINGER_SET_PRIO, 0x0105);
    // if(ret < 0)
    // {
    //     printf("failed 1 ret = %d\n", ret);
    //     return -1;
    // }

    // ret = ioctl(fd, CDFINGER_SET_PRIO, 0x0000);
    // if(ret < 0)
    // {
    //     printf("failed 2 ret = %d\n", ret);
    //     return -1;
    // }


    struct sched_param sp = {49};
    if (sched_setscheduler(0, SCHED_FIFO, &sp) < 0)
    {
        printf("failed 1\n");
        return 0;
    }


    while(1){
        t0 = cfp_get_uptime();
        for(i=0;i<10000;i++)
            for(j=0;j<10000;j++)
            {
                x++;
            }
        t1 = cfp_get_uptime();
        printf("time = %llu ms\n",t1-t0);

        int old_prio = 0;
        old_prio = getpriority(PRIO_PROCESS, 0);

        printf("old_prio = %d\n", old_prio);
    }
    return 0;
}
