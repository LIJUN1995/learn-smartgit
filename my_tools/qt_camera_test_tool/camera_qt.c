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
#include <sys/socket.h>
 #include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
// #include "savebmp.h"

sem_t g_down_sem;
typedef struct __socket_info
{
    int cmd;
    int value;
}SOCKET_INFO;

enum {
    INIT_REG,
    SET_GAIN,
    SET_EXP,
    SET_FUSION,
    WRITE_REG_VALUE,
    READ_REG_VALUE, 
    GET_IMG,
    EXIT_CMD,
};


int SENSOR_WIDTH = 0;
int SENSOR_HEIGHT = 0;
extern int write_register(unsigned short reg, unsigned char value);
extern unsigned char read_register(unsigned short reg);

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
    fd = open(FilePath, O_CREAT | O_RDWR);
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

int cfp_handler_register(int devfd, void (*func)(int))
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
    fcntl(devfd, F_SETOWN, getpid());
    fcntl(devfd, F_SETFL, fcntl(devfd, F_GETFL) | FASYNC);

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

int func_sem_timedwait()
{
    struct timespec timewait;
    timewait.tv_sec = getTimeMsec() / 1000 + 3;
    timewait.tv_nsec = 0;
    int ret = sem_timedwait(&g_down_sem, &timewait);
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

int WaitSemCall()
{
    int ret = -1;

    while ((ret = func_sem_timedwait()) == -1 && errno == EINTR)
        continue; /* Restart if interrupted by handler */

    if (ret == -1)
    {
        if (errno == ETIMEDOUT)
            printf("timeout!!!!!!!!!!!!!!!!!!!!!!!\n");
    }
    return ret;
}

void get_img(unsigned short *p, cdfinger_fops *sensor_fops)
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

    sensor_fops->sensor_pre_image();
    ret = WaitSemCall();
    if (ret == -1)
    {
        exit(1);
    }
    long long t1 = cfp_get_uptime();
    printf("sensor exp time spend %lldms\n", t1 - t0);

    sensor_fops->sensor_get_img_buffer(p, SENSOR_WIDTH, SENSOR_HEIGHT);
    printf("spi transmission spend time  %lldms\n", cfp_get_uptime() - t1);
    printf("all spend time  %lldms\n", cfp_get_uptime() - t0);
}

int DeviceInit(int devfd)
{
    int ret = 0;

    ret = sem_init(&g_down_sem, 0, 0);
    if(ret == -1){
        perror("sem_init failed"); 
    }

    ret = cfp_handler_register(devfd, sig_handler);
    if(ret == -1){
        printf("register handler failed\n");
    }

    ioctl(devfd, CDFINGER_CHANGER_CLK_FREQUENCY, 9600000);
    ioctl(devfd, CDFINGER_INIT_IRQ);
    ioctl(devfd, CDFINGER_POWER_ON);

    return ret;
}

void reset_device(int devfd)
{
    ioctl(devfd, CDFINGER_HW_RESET, 1);
    usleep(1000);
    ioctl(devfd, CDFINGER_HW_RESET, 0);
    usleep(1000);
    ioctl(devfd, CDFINGER_HW_RESET, 1);
    usleep(1000);
}

bool checkSensorId(int devfd, cdfinger_fops *sensor_fops)
{
    // fps6038_gc07s0_init(&sensor_fops);
    // if (sensor_fops.sensor_verify_id(fd) == true)
    // {
    //     printf("current sensor is fps6038_gc07s0_init");
    //     ioctl(fd, CDFINGER_CHANGER_CLK_FREQUENCY, 9600000 * 2);
    //     SENSOR_WIDTH = 176;
    //     SENSOR_HEIGHT = 216;
    //     return true;
    // }

    // reset_device();

    fps7011_gcm7s0_init(sensor_fops);
    if (sensor_fops->sensor_verify_id(devfd) == true)
    {
        ioctl(devfd, CDFINGER_CHANGER_CLK_FREQUENCY, 9600000);
        SENSOR_WIDTH = 240;
        SENSOR_HEIGHT = 160;
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

int create_socket(void)
{
    int ret = 0;
    int sockfd = 0;
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd == -1)
    {
        printf("create socket failed, tcp_socket = %d\n",sockfd);
        return sockfd;
    }

    memset(&servaddr, 0, sizeof(servaddr)); 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);             
    servaddr.sin_port = htons(1234); 
    ret = bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if(ret == -1){
        printf("bind socket failed, ret = %d\n",ret);
        goto out;
    }

    ret = listen(sockfd,10);
    if(ret != 0){
        printf("listen socket failed, ret = %d\n",ret);
        goto out;
    }

    printf("create socket success\n");

    return sockfd;
out:
    close(sockfd);
    return ret;
}



int main(int argc, char *argv[])
{
    int ret = 0;
    int devfd = 0;
    int sockfd = 0;
    int client_sockfd = 0;
    struct sockaddr_in client_addr;
    unsigned int addr_len = sizeof(client_addr);
    SOCKET_INFO *socket_info = NULL;
    unsigned short *imgbuff = NULL;
    cdfinger_fops sensor_fops = {0};

    devfd = open("/dev/fpsdev0", O_RDWR);
    if (devfd < 0)
    {
        printf("open fpsdev0 failed\n");
        return -1;
    }

    ret = DeviceInit(devfd);

    sockfd = create_socket();
    if(sockfd == -1){
        return -1;
    }

    client_sockfd = accept(sockfd, (struct sockaddr*)&client_addr, &addr_len);
    if(client_sockfd == -1){
        printf("accept socket error: %s(errno: %d)\n",strerror(errno),errno); 
        return -1;
    }else{
        printf("accept success\n");
    }

    if (checkSensorId(devfd, &sensor_fops) == false)
    {
        printf("===========read id fail===========\n");
        return -1;
    }

    socket_info = (SOCKET_INFO *)malloc(sizeof(SOCKET_INFO));
    if(socket_info == NULL){
        printf("malloc socket_info failed\n");
        return -1;
    }
    memset(socket_info,0X00,sizeof(SOCKET_INFO));

    imgbuff = (unsigned short *)malloc(SENSOR_WIDTH*SENSOR_HEIGHT*2);
    if(imgbuff == NULL){
        printf("malloc imgbuff failed\n");
        goto OUT;
    }
    memset(imgbuff,0X00,SENSOR_WIDTH*SENSOR_HEIGHT*2);

    unsigned char buf[240*160+4] = {0};

    while(1){     
        ret = recv(client_sockfd, socket_info, sizeof(SOCKET_INFO), 0);
        printf("recv msg from client,ret = %d : cmd=%d, value=%d\n", ret, socket_info->cmd,socket_info->value);
        if(ret == 0){
            printf("TCP Connection interruption\n");
            break;
        }
        // sprintf(buff,"hello, lijun", 1024);
        // ret = send(client_sockfd, buff, 1024, 0);
        // printf("send msg from client,ret = %d: buff = %s\n", ret, buff);
        switch (socket_info->cmd)
        {
        case INIT_REG:
            sensor_fops.sensor_init(devfd); 
            printf("INIT_REG\n");
            break;
        case SET_GAIN:
            sensor_fops.sensor_setImgGain(socket_info->value);
            printf("SET_GAIN  %d\n", socket_info->value);
            break;
        case SET_EXP:
            sensor_fops.sensor_setExpoTime(socket_info->value);
            printf("SET_EXP %d\n", socket_info->value);
            break;
        case SET_FUSION:
            sensor_fops.sensor_setFrameNum(socket_info->value);
            printf("SET_FUSION %d\n",socket_info->value);
            break;
        case GET_IMG:
            get_img(imgbuff, &sensor_fops);

            for (size_t i = 0; i < SENSOR_WIDTH*SENSOR_HEIGHT; i++)
            {
                buf[i+4] = imgbuff[i]>>4;
            }

            int *p = (int *)buf;
            *p = GET_IMG;
            printf("p ======== %d\n",*p);
            

            ret = send(client_sockfd,buf,SENSOR_WIDTH*SENSOR_HEIGHT+4,0);
            printf("GET_IMG\n");
            break;
        case WRITE_REG_VALUE:
            write_register((unsigned short)((socket_info->value)>>16),(unsigned char)(socket_info->value));

            unsigned short reg = (unsigned short)((socket_info->value)>>16);
            unsigned char value = (unsigned char)(socket_info->value);
            printf("WRITE_REG_VALUE 0x%04x    0x%02x\n",reg, value);
            break;
        case READ_REG_VALUE:
            ret = read_register((unsigned short)(socket_info->value));
            printf("READ_REG_VALUE 0x%04x,  0x%02x\n",socket_info->value, ret);
            socket_info->cmd = READ_REG_VALUE;
            socket_info->value = ret;
            ret = send(client_sockfd,socket_info,sizeof(SOCKET_INFO),0);
            break;      
        default:
            break;
        }
    }

OUT:
    close(client_sockfd);
    if(imgbuff != NULL)
        free(imgbuff);
    if(socket_info != NULL)
        free(socket_info);
    
    return 0;
}
