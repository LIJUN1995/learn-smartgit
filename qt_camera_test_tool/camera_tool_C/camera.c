#include <errno.h>
#include <fcntl.h>
#include <linux/in.h>
#include <semaphore.h>
#include <sensor_ctl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <math.h>
// #include "savebmp.h"
#include <sched.h>

int fd = 0;
sem_t g_down_sem;
cdfinger_fops sensor_fops = {0};

int SENSOR_WIDTH = 0;
int SENSOR_HEIGHT = 0;
int img_num = 1;

extern int fb_main(void);
extern void draw_image(int w, int h, void *image, void *finger);

int create_multi_dir(const char *path)
{
    int i;
    size_t len = strlen(path);
    char dir_path[len + 1];
    dir_path[len] = '\0';
    strncpy(dir_path, path, len);
    for (i = 0; i < (int)len; i++)
    {
        if (dir_path[i] == '/' && i > 0)
        {
            dir_path[i] = '\0';
            if (access(dir_path, 0) < 0)
            {

#ifdef _WIN32
                int ret = mkdir(dir_path);
#else
                int ret = mkdir(dir_path, 0777);
#endif
                if (ret < 0)
                {
                    printf("mkdir=%s:msg=%s\n", dir_path, strerror(errno));
                    return -1;
                }
            }
            dir_path[i] = '/';
        }
    }
    return 0;
}

int fp_SaveGrayBitmap(const char *FilePath, unsigned char *pData, int row, int colume)
{
    printf("%s path = %s", __func__, FilePath);
    create_multi_dir(FilePath);
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
    }
    close(fd);

    return 0;
}

int fp_SaveBinFile(const char *FilePath, unsigned char *pData, int row, int colume)
{
    FILE *fp = fopen(FilePath, "wb+");
    if (fp == NULL)
    {
        printf("fopen %s error", FilePath);
        return -1;
    }
    fwrite(pData, row * colume, 1, fp);
    fclose(fp);

    return 0;
}

int cfp_handler_register(void (*func)(int))
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

void sig_handler(int signo)
{
    if (signo == SIGIO)
    {
        printf("[1111]SIGIO !\n");
        sem_post(&g_down_sem);
    }
    if (signo == SIGINT)
    {
        exit(0);
    }
}

static int64_t getTimeMsec()
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

unsigned long long cfp_get_uptime()
{
    struct timeval cur;
    long long int timems = 0;
    gettimeofday(&cur, NULL);
    timems = (long long int)(cur.tv_sec * 1000 + (cur.tv_usec) / 1000);
    return timems;
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
static int spi_send_data(unsigned char *tx, unsigned char *rx, int len)
{
    cdfinger_spi_data data;
    int ret = -1;

    data.rx = rx;
    data.tx = tx;
    data.length = len;
    data.tx_length = len;
    ret = ioctl(fd, CDFINGER_SPI_WRITE_AND_READ, &data);
    if (ret != 0)
    {
        printf("spi send data is error!!!\n");
    }

    return ret;
}
int checkUpdate()
{
    uint8_t tx[8] = {0};
    uint8_t rx[8] = {0};

    memset(tx, 0x66, 8);
    memset(rx, 0x66, 8);
    tx[0] = 0x8e;
    tx[1] = 0x66;
    tx[2] = 0x66;
    tx[3] = 0x66;
    spi_send_data(tx, rx, 8);
    for (int i = 0; i < 8; i++)
    {
        printf("rx[%d]=%x ", i, rx[i]);
    }
    printf("\n");

    if (rx[3] == 0xa5)
    {
        printf("===============the module is single=================\n");
    }
    else if (rx[3] == 0xa6)
    {
        printf("===============the module is double=================\n");
    }
    else
    {
        printf("===============download program failed=================\n");
        return -1;
    }

    return 0;
}
void get_imge_sem()
{
    WaitSemCall();
}
void get_img(unsigned short *p, cdfinger_fops *info)
{
    long long t0 = 0;
    int ret;
    int semcnts = 0;
    sem_getvalue(&g_down_sem, &semcnts);
    while (semcnts >= 1)
    {
        sem_wait(&g_down_sem);
        sem_getvalue(&g_down_sem, &semcnts);
    }
    t0 = cfp_get_uptime();

    sensor_fops.sensor_pre_image();
    ret = WaitSemCall();
    if (ret == -1)
    {
        checkUpdate();
        exit(1);
    }
    long long t1 = cfp_get_uptime();
    printf("sensor exp time spend %lldms\n", t1 - t0);

    sensor_fops.sensor_get_img_buffer(p, SENSOR_WIDTH, SENSOR_HEIGHT);
    printf("spi transmission spend time  %lldms\n", cfp_get_uptime() - t1);
    printf("all spend time  %lldms\n", cfp_get_uptime() - t0);
}

int need_debug(void)
{
    unsigned char tx[8] = {0};
    unsigned char rx[8] = {0};

    tx[0] = 0xf2;
    tx[1] = 0x02;
    tx[2] = 0x04;
    tx[3] = 0x00;
    tx[4] = 0x00;
    if (spi_send_data(tx, rx, 6) < 0)
    {
        return -1;
    }
    printf("==========0x%02x\n", rx[4]);
    tx[0] = 0xf2;
    tx[1] = 0x02;
    tx[2] = 0x05;
    tx[3] = 0x00;
    tx[4] = 0x00;
    if (spi_send_data(tx, rx, 6) < 0)
    {
        return -1;
    }
    printf("==========0x%02x\n", rx[4]);

    tx[0] = 0xf2;
    tx[1] = 0x02;
    tx[2] = 0xb3;
    tx[3] = 0x00;
    tx[4] = 0x00;
    if (spi_send_data(tx, rx, 6) < 0)
    {
        return -1;
    }
    printf("read GAIN =========== 0x%02x\n", rx[4]);

    return 0;
}

void checksum(unsigned char *p)
{
    int sum;
    for (size_t i = 0; i < (SENSOR_WIDTH * SENSOR_HEIGHT); i++)
    {
        sum += p[i];
    }
    sum = sum / (SENSOR_WIDTH * SENSOR_HEIGHT);
    printf("=================sum=%d\n", sum);
}

void DeviceInit(void)
{
    sem_init(&g_down_sem, 0, 0);
    cfp_handler_register(sig_handler);
    ioctl(fd, CDFINGER_CHANGER_CLK_FREQUENCY, 9600000);
    ioctl(fd, CDFINGER_INIT_IRQ);
    ioctl(fd, CDFINGER_POWER_ON);
}

void reset_device()
{
    ioctl(fd, CDFINGER_HW_RESET, 1);
    usleep(1000);
    ioctl(fd, CDFINGER_HW_RESET, 0);
    usleep(1000);
    ioctl(fd, CDFINGER_HW_RESET, 1);
    usleep(1000);
}

bool checkSensorId()
{
 /*   fps6038_gc07s0_init(&sensor_fops);
    if (sensor_fops.sensor_verify_id(fd) == true)
    {
        printf("current sensor is fps6038_gc07s0_init");
        ioctl(fd, CDFINGER_CHANGER_CLK_FREQUENCY, 9600000 * 2);
        SENSOR_WIDTH = 176;
        SENSOR_HEIGHT = 216;
        return true;
    }*/

    // reset_device();

     fps7011_gcm7s0_init(&sensor_fops);
     if (sensor_fops.sensor_verify_id(fd) == true)
     {
         img_num = 1;
         ioctl(fd, CDFINGER_CHANGER_CLK_FREQUENCY, 9600000);
         SENSOR_WIDTH = 160;
         SENSOR_HEIGHT = 106;
         return true;
     }

    // reset_device();

    // sensor_fps6037_isf1001_init(&sensor_fops);
    // if (sensor_fops.sensor_verify_id(fd) == true)
    // {
    //     printf("current sensor is sensor_fps6037_isf1001_init");
    //     ioctl(fd, CDFINGER_CHANGER_CLK_FREQUENCY, 9600000 * 2);
    //     SENSOR_WIDTH = 176;
    //     SENSOR_HEIGHT = 176;
    //     return true;
    // }

    // reset_device();

    // fps7001_gc0403_init(&sensor_fops);
    // sensor_fops.sensor_config_process("/data/system/cdfinger/cpld.bin", fd);
    // if (sensor_fops.sensor_verify_id(fd) == true)
    // {
    //     img_num = 1;
    //     ioctl(fd, CDFINGER_CHANGER_CLK_FREQUENCY, 9600000);
    //     SENSOR_WIDTH = 96;
    //     SENSOR_HEIGHT = 72;
    //     return true;
    // }

    // reset_device();

    // fps7001_gc0403_double_init(&sensor_fops);
    // sensor_fops.sensor_config_process("/data/system/cdfinger/cpld.bin", fd);
    // if (sensor_fops.sensor_verify_id(fd) == true)
    // {
    //     img_num = 2;
    //     ioctl(fd, CDFINGER_CHANGER_CLK_FREQUENCY, 9600000);
    //     SENSOR_WIDTH = 96;
    //     SENSOR_HEIGHT = 72*2+21;
    //     return true;
    // }

    return false;
}

extern unsigned char read_register(unsigned short reg);
extern int write_register(unsigned short reg, unsigned char value);

int sensor_Gain_1(unsigned char gain) {
    unsigned char ret = 0;
    unsigned char temp_gain = 0;

    if (write_register(0x2b1, gain) < 0)
    {
        return -1;
    }

    return 0;
}

int sensor_Gain_2(unsigned char gain) {
    unsigned char ret = 0;
    unsigned char temp_gain = 0;

    if (write_register(0x2b2, gain) < 0)
    {
        return -1;
    }

    return 0;
}

int Euclidean_distance(float x, float y)
{
    int g = 0;

    g = y/x+x*y-y+x/3+y-x;

	return 0;
}

int main(int argc, char *argv[])
{
    int count = 0, ret = 0, i = 0;
    unsigned short *imgbuf1 = NULL;
    unsigned char *bmp_buf1 = NULL;
    unsigned char *bmp_buf2 = NULL;

    fd = open("/dev/fpsdev0", O_RDWR);
    if (fd < 0)
    {
        printf("open fpsdev0 fail");
        return -1;
    }

    // DeviceInit();
    // if (checkSensorId() == false)
    // {
    //     printf("===========read id fail===========\n");
    //     return -1;
    // }

    ret = fb_main();
    if(ret == -1){
        return -1;
    }

    count = atoi(argv[2]);
    sensor_fops.sensor_init(fd);
    sensor_fops.sensor_setExpoTime(atoi(argv[1]));
    sensor_fops.sensor_setBinning(2);
    sensor_fops.sensor_setImgGain(1);
    sensor_fops.sensor_setFrameNum(1);

    printf("SENSOR_WIDTH=%d,SENSOR_HEIGHT=%d\n", SENSOR_WIDTH, SENSOR_HEIGHT);
    imgbuf1 = (unsigned short *)malloc(SENSOR_WIDTH * SENSOR_HEIGHT * 2 * img_num);
    bmp_buf1 = (unsigned char *)malloc(SENSOR_WIDTH * SENSOR_HEIGHT);

    while (count--)
    {
        int sum = 0;
        // get_img(bmp_buf1, &sensor_fops);
        get_img(imgbuf1, &sensor_fops);
        for (size_t i = 0; i < (SENSOR_WIDTH * SENSOR_HEIGHT); i++)
        {
            bmp_buf1[i] = imgbuf1[i] >> 4;

            sum += bmp_buf1[i];

            // if (MODULE_TYPE == 0xa6)
            //     bmp_buf2[i] = imgbuf1[i + SENSOR_HEIGHT * SENSOR_WIDTH] >> 4;
            // printf("%d   ",imgbuf1[i]);
            // if(bmp_buf[i]!=0xc0){

            // printf("%x ",bmp_buf[i] );
            //   printf("============fail================\n");
            //    exit(0);
            //}
        }

        printf("sum ============= %d\n", sum / (SENSOR_HEIGHT * SENSOR_WIDTH));

        char path[64] = {0};
        draw_image(SENSOR_WIDTH, SENSOR_HEIGHT, bmp_buf1, NULL);
        sprintf(path, "/data/pic/%d.bmp", i);
        fp_SaveGrayBitmap(path, bmp_buf1, SENSOR_HEIGHT, SENSOR_WIDTH);
        // sprintf(path, "/data/pic/%d.bin", i);
        // fp_SaveBinFile(path, p, SENSOR_HEIGHT * 2, SENSOR_WIDTH);
        i++;
        // checksum(bmp_buf);
    }


out:
    free(imgbuf1);
    free(bmp_buf1);
    free(bmp_buf2);

    return 0;
}
