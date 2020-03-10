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
#include <math.h>
// #include "savebmp.h"

sem_t g_down_sem;
typedef struct __socket_info
{
    uint16_t cmd;
    uint16_t value1;
    uint16_t value2;
    uint16_t value3;
    uint16_t value4;
}SOCKET_INFO;

enum {
    INIT_SENSOR = 1000,
    SET_BINNING,
    SET_FUSION,
    SET_IMGWH,
    SET_GAIN,
    SET_EXP,
    WRITE_REG_VALUE,
    READ_REG_VALUE, 
    GET_IMG,
    EXIT_CMD,
    BRUSH_IMG,
    STATUS_FAILED,
    STATUS_SUCCESS,
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

unsigned short cfp_get_uptime()
{
    struct timeval cur;
    unsigned short timems = 0;
    gettimeofday(&cur, NULL);
    timems = (unsigned short)(cur.tv_sec * 1000 + (cur.tv_usec) / 1000);
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

int get_img(unsigned short *p, cdfinger_fops *sensor_fops, SOCKET_INFO *socket_info)
{
    unsigned short t0 = 0, t1 = 0, t2 = 0;
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
        return ret;
    }
    t1 = cfp_get_uptime();
    printf("sensor exp time spend %dms\n", t1-t0);

    sensor_fops->sensor_get_img_buffer(p, SENSOR_WIDTH, SENSOR_HEIGHT);
    t2 = cfp_get_uptime();
    printf("spi transmission spend time  %dms\n", t2 - t1);
    printf("all spend time  %dms\n", t2 - t0);

    socket_info->value1 = t1-t0;
    socket_info->value2 = t2-t1;
    socket_info->value3 = t2-t0;

    return 0;
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
    fps6038_gc07s0_init(sensor_fops);
    if (sensor_fops->sensor_verify_id(devfd) == true)
    {
        printf("current sensor is fps6038_gc07s0_init\n");
        ioctl(devfd, CDFINGER_CHANGER_CLK_FREQUENCY, 9600000);
        SENSOR_WIDTH = 176;
        SENSOR_HEIGHT = 216;
        return true;
    }

    reset_device(devfd);

    fps7011_gcm7s0_init(sensor_fops);
    if (sensor_fops->sensor_verify_id(devfd) == true)
    {
        ioctl(devfd, CDFINGER_CHANGER_CLK_FREQUENCY, 9600000);
        SENSOR_WIDTH = 240;
        SENSOR_HEIGHT = 160;
        return true;
    }

    // reset_device()

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

int re_malloc(void **p, int len)
{
    if(*p != NULL)
        free(p);
    *p = malloc(len);
    if(*p == NULL)
    {
        printf("malloc failed\n");
        return -1;
    }
    return 0;
}

float getPsnr(uint16_t* singleImg, uint16_t* img, int width, int height)
{
	int i;
	float N = 0;
	for (i = 0; i < width * height; ++i)
	{
		N += (singleImg[i] - img[i]) * (singleImg[i] - img[i]);
	}
	N = N / (width * height);
	return 10 * log10(4096 * 4096 / N);
}

float getSnr(uint16_t* singleImg, uint16_t* img, int width, int height)
{
	int i;
	float S = 0, N = 0;
	for (i = 0; i < width * height; ++i)
	{
		S += singleImg[i] * singleImg[i];
		N += (singleImg[i] - img[i]) * (singleImg[i] - img[i]);
	}
	return 10 * log10(S / N);
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

    socket_info = (SOCKET_INFO *)malloc(sizeof(SOCKET_INFO));
    if(socket_info == NULL){
        printf("malloc socket_info failed\n");
        return -1;
    }
    memset(socket_info,0X00,sizeof(SOCKET_INFO));

    while(1){     
        ret = recv(client_sockfd, socket_info, sizeof(SOCKET_INFO), 0);
        printf("recv msg from client,ret = %d : cmd=%d, value=%d\n", ret, socket_info->cmd,socket_info->value1);
        if(ret == 0){
            printf("TCP Connection interruption\n");
            break;
        }
        switch (socket_info->cmd)
        {
        case INIT_SENSOR:
            if (checkSensorId(devfd, &sensor_fops) == false)
            {
                printf("===========read id fail===========\n");
                ret = -1;
                break;
            }else{
                ret = re_malloc((void **)&imgbuff,SENSOR_HEIGHT*SENSOR_WIDTH*2+sizeof(SOCKET_INFO));
                if(ret == -1)
                {
                    goto OUT;
                }
            }
            ret = sensor_fops.sensor_init(devfd); 
            if(ret != -1){
                socket_info->value1 = 0;
                socket_info->value2 = 0;
                socket_info->value3 = SENSOR_WIDTH;
                socket_info->value4 = SENSOR_HEIGHT;
            }
            printf("INIT_SENSOR\n");
            break;
        case SET_GAIN:
            ret = sensor_fops.sensor_setImgGain(socket_info->value1);
            if(ret == -1){
                socket_info->value1 = STATUS_FAILED;
            }
            printf("SET_GAIN  %d\n", socket_info->value1);
            break;
        case SET_EXP:
            ret = sensor_fops.sensor_setExpoTime(socket_info->value1);
            if(ret == -1){
                socket_info->value1 = STATUS_FAILED;
            }
            printf("SET_EXP %d\n", socket_info->value1);
            break;
        case SET_FUSION:
            ret = sensor_fops.sensor_setFrameNum(socket_info->value1);
            if(ret == -1){
                socket_info->value1 = STATUS_FAILED;
            }
            printf("SET_FUSION %d\n",socket_info->value1);
            break;
        case SET_BINNING:
            ret = sensor_fops.sensor_setBinning(socket_info->value1);
            if(ret != -1){
                int temp_ret = 0;
                
                temp_ret = re_malloc((void **)&imgbuff,SENSOR_HEIGHT*SENSOR_WIDTH*2+sizeof(SOCKET_INFO));
                if(temp_ret == -1)
                {
                    goto OUT;
                }else{
                    socket_info->value3 = SENSOR_WIDTH;
                    socket_info->value4 = SENSOR_HEIGHT;
                }

            }
            printf("SET_BINNING %d\n", socket_info->value1);
            break;
        case SET_IMGWH:
            ret = sensor_fops.sensor_setImgWH(socket_info->value1,socket_info->value2,socket_info->value3,socket_info->value4);
            if(ret != -1){
                SENSOR_WIDTH = socket_info->value3;
                SENSOR_HEIGHT = socket_info->value4;
            }
        case GET_IMG:
        case BRUSH_IMG:
            ret = get_img(imgbuff+5, &sensor_fops, socket_info);
            if(ret == -1){
                break;
            }

            memcpy(imgbuff,socket_info,sizeof(SOCKET_INFO));
            unsigned char *p = (unsigned char *)(imgbuff+sizeof(SOCKET_INFO)/2);
            for (size_t i = 0; i < SENSOR_HEIGHT*SENSOR_WIDTH; i++)
            {
                p[i] = imgbuff[i+sizeof(SOCKET_INFO)/2]>>4;
            }
            
            printf("%d %d %d\n",socket_info->value1,socket_info->value2,socket_info->value3);
            
            ret = send(client_sockfd,imgbuff,SENSOR_WIDTH*SENSOR_HEIGHT+sizeof(SOCKET_INFO),0);
            printf("GET_IMG\n");
            break;
        case WRITE_REG_VALUE:
            ret = write_register(socket_info->value1,socket_info->value2);
            if(ret == -1){
                socket_info->value1 = STATUS_FAILED;
            }
            break;
        case READ_REG_VALUE:
            ret = read_register((unsigned short)(socket_info->value1));
            if(ret == -1){
                socket_info->value1 = STATUS_FAILED;
            }else{
                socket_info->cmd = READ_REG_VALUE;
                socket_info->value1 = ret;
                ret = send(client_sockfd,socket_info,sizeof(SOCKET_INFO),0); 
            }
            break;      
        default:
            printf("the TCP CMD error\n");
            break;
        }
        // printf("0x98=%d, 0x99=%d, 0x9a=%d\n",read_register(0x98),read_register(0x99),read_register(0x9a));
        // printf("0x9b=%d, 0x9c=%d, 0x9d=%d\n",read_register(0x9b),read_register(0x9c),read_register(0x9d));
        // printf("0x9e=%d, 0x9f=%d, 0xa0=%d\n",read_register(0x9e),read_register(0x9f),read_register(0xa0));
        printf("value1=%d, value2=%d, value3=%d, value4=%d\n",socket_info->value1,socket_info->value2,socket_info->value3,socket_info->value4);
        if(ret == -1){
            socket_info->value1 = STATUS_FAILED;
            ret = send(client_sockfd,socket_info,sizeof(SOCKET_INFO),0);
            if(ret == -1){
                printf("socket send error: %s(errno: %d)\n",strerror(errno),errno); 
            }
        }else if((socket_info->cmd != GET_IMG) && (socket_info->cmd != BRUSH_IMG)){
            ret = send(client_sockfd,socket_info,sizeof(SOCKET_INFO),0);
            if(ret == -1){
                printf("socket send error: %s(errno: %d)\n",strerror(errno),errno); 
            }
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
