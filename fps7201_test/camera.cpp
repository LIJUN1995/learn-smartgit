#include <iostream>
#include <cstdint>
#include <cstring>
#include <vector>

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#include <semaphore.h>
#include "fpsxxxx.h"
#include "fps7011_gcm7s0.h"
#include "BmpClass.h"

using namespace std;

int fp_SaveGrayBitmap(const char *FilePath, unsigned char *pData, int row, int colume)
{
    printf("%s path = %s", __func__, FilePath);
    int fd = 0;
    unsigned char grayBitmapHeader[1078] = {0};
    unsigned char pad[4] = {0};
    uint32_t colume_t = 0;
    uint32_t row_t = 0;
    int i = 0;
    if (NULL == pData)
        return -1;
    fd = open(FilePath, O_CREAT | O_RDWR, 0777);
    if (0 == fd)
        return -1;
    colume_t = (uint32_t)colume; //(uint32_t)(colume + 3) & 0xFFFFFFFC;
    row_t = (uint32_t)row;       // (uint32_t)(row + 3) & 0xFFFFFFFC;
    grayBitmapHeader[0] = 0x42;
    grayBitmapHeader[1] = 0x4d;
    grayBitmapHeader[2] = 0x36;
    grayBitmapHeader[3] = 0x28;
    grayBitmapHeader[4] = 0x00;
    grayBitmapHeader[5] = 0x00;
    grayBitmapHeader[6] = 0x00;
    grayBitmapHeader[7] = 0x00;
    grayBitmapHeader[8] = 0x00;
    grayBitmapHeader[9] = 0x00;
    grayBitmapHeader[10] = 0x36;
    grayBitmapHeader[11] = 0x04;
    grayBitmapHeader[12] = 0x00;
    grayBitmapHeader[13] = 0x00;
    grayBitmapHeader[14] = 0x28;
    grayBitmapHeader[15] = 0x00;
    grayBitmapHeader[16] = 0x00;
    grayBitmapHeader[17] = 0x00;
    grayBitmapHeader[18] = (colume_t & 0xFF);
    grayBitmapHeader[19] = (colume_t >> 8 & 0xFF);
    grayBitmapHeader[20] = (colume_t >> 16 & 0xFF);
    grayBitmapHeader[21] = (colume_t >> 24 & 0xFF);
    grayBitmapHeader[22] = (row_t & 0xFF);
    grayBitmapHeader[23] = (row_t >> 8 & 0xFF);
    grayBitmapHeader[24] = (row_t >> 16 & 0xFF);
    grayBitmapHeader[25] = (row_t >> 24 & 0xFF);
    grayBitmapHeader[26] = 0x01;
    grayBitmapHeader[27] = 0x00;
    grayBitmapHeader[28] = 0x08;
    grayBitmapHeader[29] = 0x00;
    for (i = 0; i < 256; i++)
    {
        grayBitmapHeader[54 + i * 4] = (uint8_t)i;
        grayBitmapHeader[54 + i * 4 + 1] = (uint8_t)i;
        grayBitmapHeader[54 + i * 4 + 2] = (uint8_t)i;
    }
    write(fd, (char *)grayBitmapHeader, 1078);
    /*Be careful , bitmap save turn is buttom to top*/
    for (i = 0; i < row; i++)
    {
        write(fd, (char *)(pData + (row - i - 1) * colume), (size_t)colume);
        if (colume_t > (uint32_t)colume)
            write(fd, (char *)pad, (size_t)(colume_t - (uint32_t)colume));
        // write(fd, (char *)(pData + i * colume), (size_t)colume);
    }
    close(fd);

    return 0;
}

sem_t g_down_sem;

void sig_handler(int signo)
{
    if (signo == SIGIO)
    {
        cout<<"[1111]SIGIO!"<<endl;
        sem_post(&g_down_sem);
    }
    if (signo == SIGINT)
    {
        exit(0);
    }
}

int cfp_handler_register(void (*func)(int),int fd)
{
    struct sigaction action;
    if (func == NULL)
    {
        printf("Func to register is NULL.\n");
        return -1;
    }
    memset(&action, 0, sizeof(action));
    action.sa_handler = func;
    action.sa_flags = 0;
    sigaction(SIGIO, &action, NULL);
    sigaction(SIGINT, &action, NULL);
    fcntl(fd, F_SETOWN, getpid());
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | FASYNC);
    return 0;
}

int64_t getTimeMsec()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int func_sem_timedwait(sem_t *g_down_sem)
{
    struct timespec timewait;
    timewait.tv_sec = getTimeMsec() / 1000 + 3;
    timewait.tv_nsec = 0;
    int ret = sem_timedwait(g_down_sem, &timewait);
   
    return ret;
}


 int WaitSemCall(void)
{
    int ret = -1;

    while ((ret = func_sem_timedwait(&g_down_sem)) == -1 && errno == EINTR)
        continue; /* Restart if interrupted by handler */

    if (ret == -1)
    {
        if (errno == ETIMEDOUT)
            printf("timeout!!!!!!!!!!!!!!!!!!!!!!!\n");
    }
    return ret;
}

int main(int argc, char *argv[])
{
    uint16_t *p;
    int ret = 0;
    vector<uint16_t> vec;
    vector<uint8_t> vec1;

    Fps7011 *fps = NULL;
    try {
        fps = new Fps7011;
    }catch (const char* msg) {
        cerr << msg << endl;
        goto OUT;
    }

    ret = sem_init(&g_down_sem, 0, 0);
    if(ret < 0)
    {
        cerr<<"sem init failed"<<endl;
        goto OUT;
    }

    cfp_handler_register(sig_handler,fps->get_private_fd());

    fps->sensor_init();
    fps->sensor_pre_image();
    WaitSemCall();
    fps->sensor_setImgWH(0,0,240,160);
    fps->sensor_get_img_buffer(vec);
    
    p = (uint16_t *)&vec[0];
    for (size_t i = 0; i < vec.size(); i++)
    for (size_t i = 0; i < 240*160; i++)
    {
        // cout<<hex<<int(p[i])<<endl;
        // vec1.push_back(uint8_t(p[i]>>4));
    }

    BmpOps bmpOps;
    bmpOps.saveGrayBitmap("0.bmp",&vec1[0],160,240);
    
OUT:
    delete fps;

    return 0;
}