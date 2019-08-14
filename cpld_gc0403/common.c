#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include "common.h"
#include "cdfinger_signal.h"

int m_fd = -1;

int OpenDeviceFile(char *dir)
{
    m_fd = open("/dev/fpsdev0", O_RDWR);
	if (m_fd < 0) {
		printf("open device failed!\n");
		return m_fd;
	}else{
        printf("open device success!\n");
    }
}

void DeviceInit(void)
{
    uint8_t prio = 0;
    fb_main();

    //ioctl(m_fd, CDFINGER_RESET);
    // scanf("%d", &prio);
    // ioctl(m_fd,CDFINGER_SET_PRIO,prio);

    ioctl(m_fd, CDFINGER_INIT_IRQ);
    ioctl(m_fd, CDFINGER_POWER_ON);
	ioctl(m_fd, CDFINGER_CHANGER_CLK_FREQUENCY, 1000000);
    sem_init(&g_down_sem, 0, 0);
    cfp_handler_register(sig_handler);
}

void CloseDeviceFile(void)
{
    close(m_fd);
}

int WaitSemCall(void)
{
    int ret = -1;
    int semcnts = 0;

    sem_getvalue(&g_down_sem, &semcnts);
    while (semcnts >= 1) {
        sem_wait(&g_down_sem);
        sem_getvalue(&g_down_sem, &semcnts);
        //printf("semcnts2 = %d\n", semcnts);
    }

    //cdfinger_camera_capture();

    while ((ret = func_sem_timedwait(&g_down_sem)) == -1 && errno == EINTR)
        continue; /* Restart if interrupted by handler */

    /* 检查发生了什么 */
    //perror("sem_timewait");
    if(ret == -1){
        if (errno == ETIMEDOUT){
            printf("timeout!!!!!!!!!!!!!!!!!!!!!!!\n");
            return ret;
        }
    }
}