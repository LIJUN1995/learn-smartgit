#include <stdio.h>
#include "cmos.h"
#include "i2c.h"
#include "spi.h"
#include <stdlib.h>
//#include "log.h"
#include <errno.h>
#include "cdfinger_signal.h"
#include "sys/ioctl.h"
#include "test_cpld.h"
#include <string.h>
#include "common.h"
#include "mipi_to_spi_impl_v18.h"

uint16_t dig = 1;
extern const unsigned char gImage_test[442368];
chip_cfg camera_cfg={
    CAMERA_CFG_STATE,//state
    EXP_TIME,//exp_time
    X,//x
    Y,//y
    WIDTH,//width
    HIGHT,//hight
    FULL_WIDTH,//full_size_w
    DO_UPDATA_PARAMETERS,//update
    DON_T_UPDATA_PROGRAM,//updata_program
};

static unsigned char SENSOR_INIT_CONF[][2] = {
    0xfe, 0x80, 0xfe, 0x80, 0xfe, 0x80, 0xf2, 0x00, 0xf6, 0x00, 0xfc, 0xc6, 0xf7, 0x19, 0xf8, 0x01, 0xf9, 0x3e, 0xfe,
    0x03, 0x06, 0x80, 0x06, 0x00, 0xfe, 0x00, 0xf9, 0x2e, 0xfe, 0x00, 0xfa, 0x00, 0xfe, 0x00, 0x03, 0x0a, 0x04, 0x55,
    0x05, 0x00, 0x06, 0xbb, 0x07, 0x00, 0x08, 0x46, 0x0c, 0x04, 0x0d, 0x02, 0x0e, 0x48, 0x0f, 0x03, 0x10, 0x08, 0x11,
    0x23, 0x12, 0x10, 0x13, 0x11, 0x14, 0x01, 0x15, 0x00, 0x16, 0xc0, 0x17, 0x14, 0x18, 0x02, 0x19, 0x38, 0x1a, 0x11,
    0x1b, 0x06, 0x1c, 0x04, 0x1d, 0x00, 0x1e, 0xfc, 0x1f, 0x09, 0x20, 0xb5, 0x21, 0x3f, 0x22, 0xe6, 0x23, 0x32, 0x24,
    0x2f, 0x27, 0x00, 0x28, 0x00, 0x2a, 0x00, 0x2b, 0x03, 0x2c, 0x00, 0x2d, 0x01, 0x2e, 0xf0, 0x2f, 0x01, 0x25, 0xc0,
    0x3d, 0xe0, 0x3e, 0x45, 0x3f, 0x1f, 0xc2, 0x17, 0x30, 0x22, 0x31, 0x23, 0x32, 0x02, 0x33, 0x03, 0x34, 0x04, 0x35,
    0x05, 0x36, 0x06, 0x37, 0x07, 0x38, 0x0f, 0x39, 0x17, 0x3a, 0x1f, 0xb6, 0x04, 0xfe, 0x00, 0x8a, 0x00, 0x8c, 0x07,
    0x8e, 0x02, 0x90, 0x01, 0x94, 0x02, 0x95, 0x02, 0x96, 0x40, 0x97, 0x03, 0x98, 0x00, 0xfe, 0x00, 0x18, 0x02, 0x40,
    0x22, 0x41, 0x01, 0x5e, 0x00, 0x66, 0x20, 0xfe, 0x03, 0x01, 0x83, 0x02, 0x11, 0x03, 0x96, 0x04, 0x01, 0x05, 0x00,
    0x06, 0xa4, 0x10, 0x90, 0x11, 0x2b, 0x12, 0xc0, 0x13, 0x03, 0x15, 0x02, 0x21, 0x10, 0x22, 0x03, 0x23, 0x20, 0x24,
    0x02, 0x25, 0x10, 0x26, 0x05, 0x21, 0x10, 0x29, 0x03, 0x2a, 0x0a, 0x2b, 0x04, 0xfe, 0x00, 0xb0, 0x50, 0xb6, 0x01,

    0x03,0x00,0x04,0x05,
    // 0x8b, 0xb2, 0x8d, 0x03, 0x8c, 0x37,

    // 0x8b, 0xa2, 0x8d, 0x03,0x8c,0x07,
};

/********************************************************************************
*函 数 名：cdfinger_camera_reset
*函数功能：配置复位模式
*输    入：
*输    出：
*备    注：
********************************************************************************/
void cdfinger_camera_reset(void){
    printf("reset camera~~~~~~~~~~\n");
    i2c_write_data(0x3007,0x01);
}

/********************************************************************************
*函 数 名：cdfinger_camera_powerdown
*函数功能：配置待机模式
*输    入：
*输    出：
*备    注：
*********************************************************************************/
void cdfinger_camera_powerdown(void){
    i2c_write_data(0x3007,0x02);
#ifdef SPI_TRANSFER
    if(i2c_read_data(0x3007) == 0x02)
        printf("enter powerdown mode~\n");
#endif
}

/********************************************************************************
*函 数 名：cdfinger_camera_work
*函数功能：配置工作模式
*输    入：
*输    出：
*备    注：
********************************************************************************/
void cdfinger_camera_work(void){
    i2c_write_data(0x3007,0x00);
#ifdef SPI_TRANSFER
    if(i2c_read_data(0x3007) == 0x00)
        printf("enter work mode~\n");
#endif
}

/********************************************************************************
*函 数 名：cdfinger_camera_master_mode
*函数功能：配置连续出图模式
*输    入：
*输    出：
*备    注：
********************************************************************************/
void cdfinger_camera_master_mode(void){
    i2c_write_data(0x301a,0x02);
    cdfinger_camera_powerdown();
    cdfinger_camera_work();
    cdfinger_camera_powerdown();
    cdfinger_camera_work();
}

/********************************************************************************
*函 数 名：cdfinger_camera_read_id
*函数功能：读取摄像头id
*输    入：
*输    出：
*备    注：
********************************************************************************/
void cdfinger_camera_read_id(void){
    char id = 0;
    while(id != 0x04)
#ifdef SPI_TRANSFER
        {
            id = i2c_read_data(0xf0);
            printf("DEVICE_ID[15:8] = 0x%02x\n",i2c_read_data(0xf0));
            printf("DEVICE_ID[7:0]  = 0x%02x\n",i2c_read_data(0xf1));
            //printf("REV_NUM[15:8]  = 0x%02x\n",i2c_read_data(0x3002));
            //printf("REV_NUM[7:0]  = 0x%02x\n",i2c_read_data(0x3003));
            if(id != 0x04){
               printf("camera sensor id read failed\n");
               exit(1);
            }
        }
#else
    ;
#endif
}

/********************************************************************************
*函 数 名：cdfinger_camera_dvp_init
*函数功能：按照dvp接口进行相关寄存器配置
*输    入：
*输    出：
*备    注：
********************************************************************************/
void cdfinger_camera_dvp_init(void){
    uint8_t i;
    uint8_t j;
    uint16_t address;
    uint8_t value = 0;

    printf("dvp regsters are setting~,count = %d\n",sizeof(M1_DVP_INIT_Conf)/(sizeof(uint16_t)*2));

    for(i = 0; i < sizeof(M1_DVP_INIT_Conf)/(sizeof(uint16_t)*2); i++){
        i2c_write_data(M1_DVP_INIT_Conf[i][0], M1_DVP_INIT_Conf[i][1]);
        printf("0x%04x = 0x%02x(0x%02x)\n",M1_DVP_INIT_Conf[i][0],i2c_read_data(M1_DVP_INIT_Conf[i][0]),M1_DVP_INIT_Conf[i][1]);
        while(i2c_read_data(M1_DVP_INIT_Conf[i][0]) != M1_DVP_INIT_Conf[i][1]){
            printf("init error====>0x%04x = 0x%02x(0x%02x)\n",M1_DVP_INIT_Conf[i][0],i2c_read_data(M1_DVP_INIT_Conf[i][0]),M1_DVP_INIT_Conf[i][1]);
            break;
        }
    }
}

/********************************************************************************
*函 数 名：cdfinger_camera_pll
*函数功能：设置摄像头的时钟
*输    入：mclk, pclk单位为MHZ
*输    出：
*备    注：
********************************************************************************/
void cdfinger_camera_pll_mclk20M_pclk21_4M(void){
    i2c_write_data(0x3300,0x3c);
    i2c_write_data(0x3301,0x00);
    i2c_write_data(0x3302,0x07);
    i2c_write_data(0x3303,0x04);

    printf("0x3300 = 0x%02x\n",i2c_read_data(0x3300));
    printf("0x3301 = 0x%02x\n",i2c_read_data(0x3301));
    printf("0x3302 = 0x%02x\n",i2c_read_data(0x3302));
    printf("0x3303 = 0x%02x\n",i2c_read_data(0x3303));
}

/********************************************************************************
*函 数 名：cdfinger_camera_full_frame_size
*函数功能：设置和帧率相关的整幅图大小的窗口设置
*输    入：row行的大小有多少字节，column列的大小有多少字节
*输    出：
*备    注：
********************************************************************************/
void cdfinger_camera_full_frame_size(int row, int column){
    uint8_t data;
    uint8_t value = 0;
#ifdef SPI_TRANSFER
    printf("full_frame_size regsters are setting~\n");
#endif
    /*列的size大小配置*/
    data = (uint8_t)((column & 0xff00)>>8);
    i2c_write_data(0x3200, data);
#ifdef SPI_TRANSFER
    value = i2c_read_data(0x3200);
    if(value == data)
        printf("register address = 0x%04x, register data = 0x%02x\n",0x3200,value);
    else
        printf("register address = 0x%04x, register data = 0x%02x(0x%02x)\n",0x3200,value,data);
    
#endif    
    data = (uint8_t)(column & 0x00ff);
    i2c_write_data(0x3201, data);
#ifdef SPI_TRANSFER
    value = i2c_read_data(0x3201);
    if(value == data)
        printf("register address = 0x%04x, register data = 0x%02x\n",0x3201,value);
    else
        printf("register address = 0x%04x, register data = 0x%02x(0x%02x)\n",0x3201,value,data);
#endif

    /*行的size大小配置*/
    data = (uint8_t)((row & 0xff00)>>8);
    i2c_write_data(0x3202, data);
#ifdef SPI_TRANSFER
    value = i2c_read_data(0x3202);
    if(value == data)
        printf("register address = 0x%04x, register data = 0x%02x\n",0x3202,value);
    else
        printf("register address = 0x%04x, register data = 0x%02x(0x%02x)\n",0x3202,value,data);
#endif    
    data = (uint8_t)(row & 0x00ff);
    i2c_write_data(0x3203, data);
#ifdef SPI_TRANSFER
    value = i2c_read_data(0x3203);
    if(value == data)
        printf("register address = 0x%04x, register data = 0x%02x\n",0x3203,value);
    else
        printf("register address = 0x%04x, register data = 0x%02x(0x%02x)\n",0x3203,value,data);
#endif
}

/********************************************************************************
*函 数 名：cdfinger_camera_out_frame_size
*函数功能：设置配置的整图基础上进行裁剪的窗口设置
*输    入：row_s行开始坐标，row_e行结束坐标，column_s列开始坐标column_e列结束坐标
*输    出：
*备    注：
********************************************************************************/
void cdfinger_camera_out_frame_size(int row_s, int row_e, int column_s, int column_e){
    uint8_t data;
    uint8_t value;
#ifdef SPI_TRANSFER
    printf("out_frame_size regsters are setting~\n");
#endif
    /*列的起始坐标位置配置*/
    data = (uint8_t)((column_s & 0xff00)>>8);
    i2c_write_data(0x3204, data);
#ifdef SPI_TRANSFER
    value = i2c_read_data(0x3204);
    if(value == data)
        printf("register address = 0x%04x, register data = 0x%02x\n",0x3204,value);
    else
        printf("register address = 0x%04x, register data = 0x%02x(0x%02x)\n",0x3204,value,data);
#endif    
    data = (uint8_t)(column_s & 0x00ff);
    i2c_write_data(0x3205, data);
#ifdef SPI_TRANSFER
    value = i2c_read_data(0x3205);
    if(value == data)
        printf("register address = 0x%04x, register data = 0x%02x\n",0x3205,value);
    else
        printf("register address = 0x%04x, register data = 0x%02x(0x%02x)\n",0x3205,value,data);
#endif
    /*列的结束坐标位置配置*/
    data = (uint8_t)((column_e & 0xff00)>>8);
    i2c_write_data(0x3206, data);
#ifdef SPI_TRANSFER
    value = i2c_read_data(0x3206);
    if(value == data)
        printf("register address = 0x%04x, register data = 0x%02x\n",0x3206,value);
    else
        printf("register address = 0x%04x, register data = 0x%02x(0x%02x)\n",0x3206,value,data);
#endif    
    data = (uint8_t)(column_e & 0x00ff);
    i2c_write_data(0x3207, data);
#ifdef SPI_TRANSFER
    value = i2c_read_data(0x3207);
    if(value == data)
        printf("register address = 0x%04x, register data = 0x%02x\n",0x3207,value);
    else
        printf("register address = 0x%04x, register data = 0x%02x(0x%02x)\n",0x3207,value,data);
#endif

    /*行的起始坐标位置配置*/
    data = (uint8_t)((row_s & 0xff00)>>8);
    i2c_write_data(0x3208, data);
#ifdef SPI_TRANSFER
    value = i2c_read_data(0x3208);
    if(value == data)
        printf("register address = 0x%04x, register data = 0x%02x\n",0x3208,value);
    else
        printf("register address = 0x%04x, register data = 0x%02x(0x%02x)\n",0x3208,value,data);
#endif    
    data = (uint8_t)(row_s & 0x00ff);
    i2c_write_data(0x3209, data);
#ifdef SPI_TRANSFER
    value = i2c_read_data(0x3209);
    if(value == data)
        printf("register address = 0x%04x, register data = 0x%02x\n",0x3209,value);
    else
        printf("register address = 0x%04x, register data = 0x%02x(0x%02x)\n",0x3209,value,data);
#endif
    /*行的结束坐标位置配置*/
    data = (uint8_t)((row_e & 0xff00)>>8);
    i2c_write_data(0x320a, data);
#ifdef SPI_TRANSFER
    value = i2c_read_data(0x320a);
    if(value == data)
        printf("register address = 0x%04x, register data = 0x%02x\n",0x320a,value);
    else
        printf("register address = 0x%04x, register data = 0x%02x(0x%02x)\n",0x320a,value,data);
#endif    
    data = (uint8_t)(row_e & 0x00ff);
    i2c_write_data(0x320b, data);
#ifdef SPI_TRANSFER
    value = i2c_read_data(0x320b);
    if(value == data)
        printf("register address = 0x%04x, register data = 0x%02x\n",0x320b,value);
    else
        printf("register address = 0x%04x, register data = 0x%02x(0x%02x)\n",0x320b,value,data);
#endif
    
}

/********************************************************************************
*函 数 名：cdfinger_camera_exp_gain
*函数功能：设置配置的裁剪图的曝光时间的增益
*输    入：gpa增益大小 adc增益大小增益大小留几个固定搭配
*输    出：
*备    注：
********************************************************************************/
void cdfinger_camera_exp_gain(uint8_t gpa, uint8_t adc){
    uint8_t value = 0;
    i2c_write_data(0x3102, (uint8_t)gpa);
    //i2c_write_data(0x3103, (uint8_t)adc);
#ifdef SPI_TRANSFER
    value = i2c_read_data(0x3102);
    if(value == gpa)
        printf("register address = 0x%04x, register data = 0x%02x\n",0x3102,value);
    else
        printf("register address = 0x%04x, register data = 0x%02x(0x%02x)\n",0x3102,value,gpa);
#endif
}

/********************************************************************************
*函 数 名：cdfinger_camera_capture
*函数功能：获得一帧图像
*输    入：
*输    出：
*备    注：
********************************************************************************/
void cdfinger_camera_capture(void){
#ifdef SPI_TRANSFER
    printf("get one picture~\n");
#endif
    while(i2c_read_data(0x3013) != 0x02){
       printf("0x3013 = 0x%02x \n",i2c_read_data(0x3013)); 
        usleep(200*1000);
    }

    printf("0x3013 = 0x%02x\n", i2c_read_data(0x3013));
    i2c_write_data(0x301b, 0x01);
    //printf("0x301b =　0x%02x\n",i2c_read_data(0x301b));
}

/********************************************************************************
*函 数 名：cdfinger_camera_test_mode
*函数功能：开关test mode
*输    入：ifopenornot：开启OPEN_TEST_MODE或者关闭CLOSE_TEST_MODE的参数
*输    出：
*备    注：
********************************************************************************/
void cdfinger_camera_test_mode(uint8_t ifopenornot){
#ifdef SPI_TRANSFER
    if(ifopenornot == 0x00)
        printf("close test mode~\n");
    else if(ifopenornot == 0x03 || ifopenornot == 0x01)
        printf("open test mode~\n");
    else
        printf("wrong input to set test mode~\n");
#endif
    i2c_write_data(0x3600, ifopenornot);
    i2c_write_data(0x3601, (uint8_t)((dig>>8)&0x00ff));
    i2c_write_data(0x3602, (uint8_t)(dig&0x00ff));
    i2c_write_data(0x3603, 0x00);
    i2c_write_data(0x3604, 0x00);
    printf("0x3601 = 0x%02x\n",i2c_read_data(0x3601));
    printf("0x3602 = 0x%02x\n",i2c_read_data(0x3602));
    printf("0x3603 = 0x%02x\n",i2c_read_data(0x3603));
    printf("0x3604 = 0x%02x\n",i2c_read_data(0x3604));
    
}

/********************************************************************************
*函 数 名：cdfinger_change_exposureTime
*函数功能：配置增益
*输    入：
*输    出：
*备    注：
********************************************************************************/
void cdfinger_change_exposureTime(int i)
{
	uint8_t np = 0,nb = 0;
	uint32_t temp = i*25714/FULL_WIDTH;
    printf("temp = %d\n",temp);
	np=temp>>8&0xff;
	nb=(uint8_t)temp&0xff;
#ifdef SPI_TRANSFER
	printf("np = 0x%02x,nb = 0x%02x\n",np,nb);
#endif
	i2c_write_data(0x3100,np);
	i2c_write_data(0x3101,nb);	
}

void update_background_image(void){
    
}

struct m_bit{
    unsigned long a:10;
    unsigned long b:10;
    unsigned long c:10;
    unsigned long d:10;
}*m_bit_p;

#ifdef PRINTF_ERROR_PIXEL
struct img_error_data {
    uint32_t img_num;
    uint32_t pixel_num;
    unsigned char data_rx;
    uint16_t img_data;
};
#endif

static int spi_send_data_m(unsigned char *tx, unsigned char *rx, int len)
{
    int ret = -1;
    cdfinger_spi_data data;

    data.tx = tx;
    data.rx = rx;
    data.length = len;
    data.tx_length = len;

    ret = ioctl(m_fd, CDFINGER_SPI_WRITE_AND_READ, &data);
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
    spi_send_data_m(tx,rx,8);
    for (int i = 0; i < 8; i++)
    {
        printf("rx[%d]=%x ", i, rx[i]);
    }
    printf("\n");

    if (rx[4] != 0x20)
    {
        printf("==============downloader program fail=================\n");
    }

    return 0;
}

void close_cpld1(void){
    uint8_t tx[8] = {0};
    uint8_t rx[8] = {0};

    memset(tx, 0x66, 8);
    memset(rx, 0x66, 8);
    tx[0] = 0x8d;
    tx[1] = 0x20;
    tx[2] = 0x66;
    tx[3] = 0x66;
    spi_send_data_m(tx,rx,4);
    
    return;
}

void close_cpld2(void)
{
    uint8_t tx[8] = {0};
    uint8_t rx[8] = {0};

    memset(tx, 0x66, 8);
    memset(rx, 0x66, 8);
    tx[0] = 0x8d;
    tx[1] = 0x10;
    tx[2] = 0x66;
    tx[3] = 0x66;
    spi_send_data_m(tx,rx,4);
    
    return;
}

int sensor_setExpoTime(int time)
{
    int frame_time = 0;
    int VB = 0X46;
    int HB = 0Xbb;
    unsigned char vb1 = 0, vb2 = 0;
    unsigned char hb1 = 0, hb2 = 0;
    float row_time = 0;
    int exp = 0;

    unsigned char reg_03 = 0, reg_04 = 0;

    row_time = (406 + HB) / 10000.00;
    exp = time / row_time;

    printf("exp == %d\n", exp);

    if (exp <= 774 + VB)
    {
        frame_time = 774 + VB;
    }
    else
    {
        frame_time = exp * row_time;
    }

    reg_03 = exp >> 8;
    reg_04 = exp;
    printf("reg_03=0x%02x, reg_04=0x%02x\n", reg_03, reg_04);

    if (i2c_write_data(0xfe, 0x00) != 0)
    {
        printf("i2c write 0x03 error\n");
        return -1;
    }

    if (i2c_write_data(0x03, reg_03) != 0)
    {
        printf("i2c write 0x03 error\n");
        return -1;
    }
    if (i2c_write_data(0x04, reg_04) != 0)
    {
        printf("i2c write 0x04 error\n");
        return -1;
    }
    // usleep(1000*80);
    // reg_03 = i2c_read_data(0x03);
    // reg_04 = i2c_read_data(0x04);
    // printf("reg_03=0x%02x, reg_04=0x%02x\n", reg_03, reg_04);

    return 0;
}

int get_pictures(void)
{
    int p_count=0;

    int i=0,j=0,ret=0;
    cdfinger_spi_data data;
    unsigned short *img_data = (unsigned short *)malloc(2*WIDTH*HIGHT);
    unsigned char *img = (unsigned char *)malloc(WIDTH*HIGHT);
    data.rx = (unsigned char *)malloc(CMOS_BUFFER_SIZE);
    data.tx = (unsigned char *)malloc(CMOS_BUFFER_SIZE);

    printf("CMOS_BUFFER_SIZE = %d\n",CMOS_BUFFER_SIZE);

    printf("how many pictures do you want:");
    scanf("%d",&p_count);
    // p_count = 1;
    // int exp_time = 0;
    // scanf("%d",&exp_time);
    // sensor_setExpoTime(exp_time);
    while(p_count--){
        data.tx[0] = 0x8d;
        data.tx[1] = 0x40;
        data.tx[2] = 0x66;
        data.tx[3] = 0x66;
        data.length = 4;
        data.tx_length = 4;
        spi_send_data(&data);

        // t0 = cfp_get_uptime(); 

        ret = WaitSemCall();
        if(ret == -1){
            checkUpdate();
            break;
        }


        memset(data.tx, 0x66, CMOS_BUFFER_SIZE);
        memset(data.rx, 0x00, CMOS_BUFFER_SIZE);
        data.tx[0]     = 0x8a;
        data.length    = CMOS_BUFFER_SIZE;
        data.tx_length = CMOS_BUFFER_SIZE;
        spi_send_data(&data);
        // t0 = cfp_get_uptime() - t0;
        // SUM_time += t0;
        // if(t0>t1)
        //     t1 = t0;
        // if(t0 < t2)
        //     t2 = t0;
        printf("pcount=%d\n",p_count);
        j = 0;
        for (i = 0; i < (CMOS_BUFFER_SIZE - 3) / 5; ++i)
        {
            m_bit_p = (struct m_bit *)(data.rx + 3 + i * 5);
            img_data[j] = (unsigned short)(m_bit_p->a);
            img_data[j + 1] = (unsigned short)(m_bit_p->b);
            img_data[j + 2] = (unsigned short)(m_bit_p->c);
            img_data[j + 3] = (unsigned short)(m_bit_p->d);
            j += 4;
        }
       
        for(i=0;i<WIDTH*HIGHT;i++){
            img[i] = (uint8_t)(img_data[i]>>2);
        }
        draw_image(WIDTH,HIGHT,img,NULL);
    }

    free(data.rx);
    free(data.tx);
    free(img_data);

    return 0;
}

int get_two_picture(void){
    int p_count=0,img_num=0,err_num=0;

    int i=0,j=0,ret=0;
    long long t0=0,t1=0,t2=0;
    cdfinger_spi_data data;
    unsigned short *img_data = (unsigned short *)malloc(2*WIDTH*HIGHT);
    unsigned char *img = (unsigned char *)malloc(WIDTH*HIGHT);
    data.rx = (unsigned char *)malloc(CMOS_BUFFER_SIZE);
    data.tx = (unsigned char *)malloc(CMOS_BUFFER_SIZE);

    printf("CMOS_BUFFER_SIZE = %d\n",CMOS_BUFFER_SIZE);

    printf("how many pictures do you want:");
    scanf("%d",&p_count);
    // p_count = 1;
   
    while(p_count--){
        data.tx[0] = 0x8d;
        data.tx[1] = 0x40;
        data.tx[2] = 0x66;
        data.tx[3] = 0x66;
        data.length = 4;
        data.tx_length = 4;
        spi_send_data(&data);

        t0 = cfp_get_uptime(); 

        ret = WaitSemCall();
        if(ret == -1){
            checkUpdate();
            err_num++;
            break;
        }
        t1 = cfp_get_uptime();

        memset(data.tx, 0x66, CMOS_BUFFER_SIZE);
        memset(data.rx, 0x00, CMOS_BUFFER_SIZE);
        data.tx[0]     = 0x8a;
        data.length    = CMOS_BUFFER_SIZE;
        data.tx_length = CMOS_BUFFER_SIZE;
        spi_send_data(&data);
        t2 = cfp_get_uptime();
        printf("first image, exp_time=%ldms, spi_transfer_time=%ldms\n",t1-t0,t2-t1);
        j = 0;
        for (i = 0; i < (CMOS_BUFFER_SIZE - 3) / 5; ++i)
        {
            m_bit_p = (struct m_bit *)(data.rx + 3 + i * 5);
            img_data[j] = (unsigned short)(m_bit_p->a);
            img_data[j + 1] = (unsigned short)(m_bit_p->b);
            img_data[j + 2] = (unsigned short)(m_bit_p->c);
            img_data[j + 3] = (unsigned short)(m_bit_p->d);
            j += 4;
        }
       
        for(i=0;i<WIDTH*HIGHT;i++){
            img[i] = (uint8_t)(img_data[i]>>2);
        }
        draw_image(WIDTH,HIGHT,img,NULL);

        // usleep(1000*100);

        t0 = cfp_get_uptime(); 
        close_cpld1();

        memset(data.tx, 0x66, CMOS_BUFFER_SIZE);
        memset(data.rx, 0x00, CMOS_BUFFER_SIZE);
        data.tx[0]     = 0x8a;
        data.length    = CMOS_BUFFER_SIZE;
        data.tx_length = CMOS_BUFFER_SIZE;
        spi_send_data(&data);
        t1 = cfp_get_uptime(); 
        printf("second image,spi_transfer_time=%ldms\n",t1-t0);
        j = 0;
        for (i = 0; i < (CMOS_BUFFER_SIZE - 3) / 5; ++i)
        {
            m_bit_p = (struct m_bit *)(data.rx + 3 + i * 5);
            img_data[j] = (unsigned short)(m_bit_p->a);
            img_data[j + 1] = (unsigned short)(m_bit_p->b);
            img_data[j + 2] = (unsigned short)(m_bit_p->c);
            img_data[j + 3] = (unsigned short)(m_bit_p->d);
            j += 4;
        }
       
        for(i=0;i<WIDTH*HIGHT;i++){
            img[i] = (uint8_t)(img_data[i]>>2);
        }
        draw_image(WIDTH,HIGHT,img,NULL);

        close_cpld2();
        // usleep(1000*500);
        printf("img_num=%d,     err_num=%d\n",++img_num,err_num);
    }

    free(data.rx);
    free(data.tx);
    free(img_data);

    return 0;
}

void move_picture(void){
    
}

static void send_clk(int clk_count)
{
    for (int i = 0; i < clk_count; ++i)
    {
        ioctl(m_fd, CDFINGER_CONTROL_CLK, 0);
        usleep(1000);
        ioctl(m_fd, CDFINGER_CONTROL_CLK, 1);
        usleep(1000);
    }
}

static int sensor_setFrameNum(uint32_t count)
{
    unsigned char tx[4] = {0};
    unsigned char rx[4] = {0};

    tx[0] = 0x90;
    tx[1] = count;
    tx[2] = 0x66;
    tx[3] = 0x66;
    if (spi_send_data_m(tx, rx, 4) < 0)
    {
        return -1;
    }

    return 0;
}

static int sensor_init(void)
{
    /*read id*/
    unsigned char tx[10] = {0};
    unsigned char rx[10] = {0};
    int reg_count = 0;
    int ret = 0;
    int i = 0;

    //////////////////////   reset cpld   //////////////////////
    tx[0] = 0xf0;
    tx[1] = 0x66;
    tx[2] = 0x66;
    tx[3] = 0x66;
    if (spi_send_data_m(tx, rx, 4) < 0)
    {
        return -1;
    }

    tx[0] = 0x8d;
    tx[1] = 0x00;
    tx[2] = 0x66;
    tx[3] = 0x66;
    if (spi_send_data_m(tx, rx, 4) < 0)
    {
        return -1;
    }

    /*set camera*/
    //////////////////////   IRQ enable   //////////////////////
    tx[0] = 0x88;
    tx[1] = 0x66;
    tx[2] = 0x66;
    tx[3] = 0x66;
    if (spi_send_data_m(tx, rx, 4) < 0)
    {
        return -1;
    }

    //////////////////////   IRQ clear   //////////////////////
    tx[0] = 0x89;
    tx[1] = 0x66;
    tx[2] = 0x66;
    if (spi_send_data_m(tx, rx, 3) < 0)
    {
        return -1;
    }

    // unsigned char sensor_id = 0;
    // sensor_id = i2c_read_data(0xf0);
    // printf("sensor id = 0x%02x\n",sensor_id);
    // if ((ret < 0) || (sensor_id != 0x04))
    // {
    //     printf("fps7001_gc0403 read id failed! ret = %d, id = 0x%02x\n", ret, sensor_id);
    //     return -1;
    // }

    reg_count = sizeof(SENSOR_INIT_CONF) / 2;
    printf("the reg_count = %d\n", reg_count);
    for (i = 0; i < reg_count; i++)
    {
        if (i2c_write_data(SENSOR_INIT_CONF[i][0], SENSOR_INIT_CONF[i][1]) == -1)
        {
            printf("i2c write data error\n");
            return -1;
        }
    }

    tx[0] = 0x88;
    tx[1] = 0x66;
    tx[2] = 0x66;
    tx[3] = 0x66;
    if (spi_send_data_m(tx, rx, 4) < 0)
    {
        return -1;
    }

    tx[0] = 0x91;
    tx[1] = 0x01;
    tx[2] = 0x66;
    tx[3] = 0x66;
    tx[4] = 0x66;
    tx[5] = 0x66;
    tx[6] = 0x66;
    tx[7] = 0x66;
    if (spi_send_data_m(tx, rx, 8) < 0)
    {
        return -1;
    }

    tx[0] = 0x92;
    tx[1] = 0x01;
    tx[2] = 0x66;
    tx[3] = 0x66;
    tx[4] = 0x66;
    tx[5] = 0x66;
    tx[6] = 0x66;
    tx[7] = 0x66;
    if (spi_send_data_m(tx, rx, 8) < 0)
    {
        return -1;
    }

    // sensor_setFrameNum(4);

    return 0;
}

void cmos_init(void){
   int ret = sensor_init();
   if(ret < 0){
       printf("sensor_init failed!!!!!!!!\n");
       return;
   }
}

void ProgrammingRawCode(void)
{
 /*   CheckIdCode();
    EnableBP();
    EraseInternalFlash();
    ResetFlashAddress();
    ProgramInternalFlash();
    ProgramUserCode();
    ResetFlashAddress();
    //SPI_Verify();
    ReadUserCode();
    WriteFeatureRow();
    ProgramDoneBit();
    usleep(10*1000);
    Bypass();
    ISC_Disable();


    Bypass();
    Resfresh();
    Bypass();
    */
  // ioctl(m_fd,CDFINGER_SPI_ACTIVE);
 //  usleep(200*1000);
  //  ioctl(m_fd,CDFINGER_SPI_ACTIVE);


    // send_sclk(100);
    // ioctl(m_fd,CDFINGER_CONTROL_CS,0);
    // ioctl(m_fd,CDFINGER_CONTROL_CLK,0);
    // ioctl(m_fd,CDFINGER_HW_RESET,0);
    // usleep(100);
    // ioctl(m_fd,CDFINGER_CONTROL_CS,1);
    // ioctl(m_fd,CDFINGER_CONTROL_CLK,1);
    // ioctl(m_fd,CDFINGER_HW_RESET,1);


    int i = 0, length = 0, ret = -1;
    unsigned char *tx = NULL;
    unsigned char *rx = NULL;

    FILE *fp = fopen("/data/cpld.bin", "rb+");
    if (fp == NULL)
    {
        printf("open file error \n");
        goto out;
    }
    fseek(fp, 0L, SEEK_END);
    length = ftell(fp);
    printf("bin file length = %d \n", length);
    fseek(fp, 0L, SEEK_SET);
    tx = (unsigned char *)malloc(length);
    if (tx == NULL)
    {
        printf("malloc failed,data.tx==FULL!!!\n");
        goto out;
    }
    rx = (unsigned char *)malloc(length);
    if (rx == NULL)
    {
        printf("malloc failed,data.rx==FULL!!!\n");
        goto out;
    }
    ioctl(m_fd, CDFINGER_INIT_GPIO, 0);
    ioctl(m_fd, CDFINGER_HW_RESET, 0);
    ioctl(m_fd, CDFINGER_CONTROL_CS, 0);
    ioctl(m_fd, CDFINGER_CONTROL_CLK, 1);
    usleep(1000);
    ioctl(m_fd, CDFINGER_HW_RESET, 1);
    usleep(1000);
    ioctl(m_fd, CDFINGER_CONTROL_CS, 1);

    send_clk(8);
    
    ioctl(m_fd, CDFINGER_INIT_GPIO,1);
    ioctl(m_fd, CDFINGER_SPI_MODE, 3);

    ret = fread(tx, sizeof(uint8_t), length, fp);
    if (ret != length)
    {
        printf("fread error!!!!!!!!!!!!!!!!!!!!!!!\n");
        ret = -1;
        goto out;
    }

    if (spi_send_data_m(tx, rx, length) < 0)
    {
        ret = -1;
        goto out;
    }

    ioctl(m_fd, CDFINGER_INIT_GPIO, 0);
    send_clk(200);

    ioctl(m_fd, CDFINGER_INIT_GPIO,1);
    ioctl(m_fd, CDFINGER_SPI_MODE, 0);
    checkUpdate();

    ret = 0;
out:
    if (tx != NULL)
        free(tx);
    if (rx != NULL)
        free(rx);
    if (fp != NULL)
        fclose(fp);
    return;
}

void check_busy(void)
{
    cdfinger_spi_data data;
    unsigned char tx[2] = {0};
    unsigned char rx[2] = {1};

    data.tx = tx;
    data.rx = rx;
    do{
        usleep(1000*2);
        data.tx[0] = 0x05;
        data.length = 2;
        data.tx_length = 2;
        spi_send_data(&data);
        printf("data.rx[0]=0x%02x, data.rx[1]=0x%02x\n",data.rx[0],data.rx[1]);
    }while((data.rx[1]&0xc1) != 0x00);
    printf("the status is success!!!\n");
    return;
}

#define SIZE_T (104960+13)

void read_nvcm_file(cdfinger_spi_data *data, char *nvcm_path)
{
    // int num = 0,ret = 0;
    // FILE *fp=fopen(nvcm_path,"rb+");
    // if(fp == NULL){
    //     printf("open file error\n");
    //     exit(1);
    // }

    // fseek(fp,0L,SEEK_END);
    // num = ftell(fp);
    // printf("bin file name=%s ,num = %d\n",nvcm_path,num);
    // fseek(fp,0L,SEEK_SET);
    // ret = fread(data->tx,sizeof(uint8_t),num,fp);
    // if(ret != num){
    //     printf("fread error!!!!!!!!!!!!!!!!!!!!!!!\n");
    //     exit(1);
    // }

    return;
}

void PowerUpAndConnectNVCM(unsigned char spi_mode)
{
    cdfinger_spi_data data;
    unsigned char tx[6] = {0};
    unsigned char rx[6] = {0};

    data.tx = tx;
    data.rx = rx;

    ioctl(m_fd,CDFINGER_CONTROL_CS,0);
    usleep(1000*102);
    ioctl(m_fd,CDFINGER_HW_RESET,0);
    usleep(1000*104);
    ioctl(m_fd,CDFINGER_CONTROL_CS,1);
    usleep(195);
    ioctl(m_fd,CDFINGER_CONTROL_CS,0);
    usleep(101);
    ioctl(m_fd,CDFINGER_HW_RESET,1);

    usleep(1000*104);

    ioctl(m_fd,CDFINGER_INIT_GPIO);
    ioctl(m_fd,CDFINGER_SPI_MODE,spi_mode);

    data.tx[0] = 0x7e;
    data.tx[1] = 0xaa;
    data.tx[2] = 0x99;
    data.tx[3] = 0x7e;
    data.tx[4] = 0x01;
    data.tx[5] = 0x0e;
    data.length = 6;
    data.tx_length = 6;
    spi_send_data(&data);

    check_busy();
}

void VerifyChipID(void)
{
    cdfinger_spi_data data;
    unsigned char tx[14] = {0};
    unsigned char rx[14] = {0};

    data.tx = tx;
    data.rx = rx;

    data.tx[0] = 0x83;
    data.tx[3] = 0x25;
    data.tx[4] = 0x20;
    data.length = 5;
    data.tx_length = 5;
    spi_send_data(&data);

    usleep(8);
    check_busy();

    memset(data.tx,0,14);
    data.tx[0] = 0x03;
    data.length = 14;
    data.tx_length = 14;
    spi_send_data(&data);

    if(data.rx[13] == 0x00){
        printf("the NVCM is not programmed!!!\n");
    }else{
        printf("the NVCM is programmed,device_id=0x%02x!!!\n",data.rx[13]);
        exit(1);
    }

    memset(data.tx,0,14);
    data.tx[0] = 0x83;
    data.tx[3] = 0x25;
    data.length = 5;
    data.tx_length = 5;
    spi_send_data(&data);

    check_busy();
}

void SetupReadingParameter(void)
{
    cdfinger_spi_data data;
    unsigned char tx[12] = {0};
    unsigned char rx[12] = {0};

    data.tx = tx;
    data.rx = rx;

    memset(data.tx,0x00,12);
    data.tx[0] = 0x82;
    data.tx[3] = 0x20;
    data.tx[8] = 0xc4;
    data.length = 12;
    data.tx_length = 12;
    spi_send_data(&data);

    usleep(8);
    check_busy();
}

void BlankCheckOnNVCM_OTP_Block(void)
{
    cdfinger_spi_data data;
    unsigned char tx[21] = {0};
    unsigned char rx[21] = {0};

    data.tx = tx;
    data.rx = rx;

    memset(data.tx,0,21);
    data.tx[0] = 0x83;
    data.tx[3] = 0x25;
    data.tx[4] = 0x10;
    data.length = 5;
    data.tx_length = 5;
    spi_send_data(&data);

    usleep(8);
    check_busy();

    memset(data.tx,0,21);
    data.tx[0] = 0x03;
    data.tx[3] = 0x20;
    data.length = 21;
    data.tx_length = 21;
    spi_send_data(&data);

    int i = 0;
    for (i = 13; i < 21; ++i)
    {
        if(data.rx[i] != 0x00){
             printf("Blank Check on NVCM Trim_Parameter_OTP Block is error!!!\n");
             exit(1);
        }
    }
    
    usleep(8);

    memset(data.tx,0,21);
    data.tx[0] = 0x83;
    data.tx[3] = 0x25;
    data.length = 5;
    data.tx_length = 5;
    spi_send_data(&data);

    check_busy();
}

void BlankCheckOnNVCMMainMemoryBlock(void)
{
    cdfinger_spi_data data;
    data.tx = (unsigned char *)malloc(SIZE_T);
    data.rx = (unsigned char *)malloc(SIZE_T);

    memset(data.tx,0,SIZE_T);
    data.tx[0] = 0x03;
    data.length = SIZE_T;
    data.tx_length = SIZE_T;
    spi_send_data(&data);

    int i = 0;
    for(i=13;i<SIZE_T;++i){
        if(data.rx[i] != 0x00){
            printf("nvcm arrays error [%d]=0x%02x\n",i,data.rx[i]);
            goto out;
        }
    }

    usleep(8);

    free(data.rx);
    free(data.tx);
    return;
out:
    free(data.rx);
    free(data.tx);
    exit(1);
}

void SetupProgrammingParameter(void)
{
    cdfinger_spi_data data;
    unsigned char tx[12] = {0};
    unsigned char rx[12] = {0};

    data.tx = tx;
    data.rx = rx;

    memset(data.tx,0x00,12);
    data.tx[0] = 0x82;
    data.tx[3] = 0x20;
    data.tx[5] = 0x15;
    data.tx[6] = 0xf2;
    data.tx[7] = 0xf0;
    data.tx[8] = 0xc2;
    data.length = 12;
    data.tx_length = 12;
    spi_send_data(&data);

    usleep(8);
    check_busy();
}

void ProgramNVCMMainMermory(void)
{
    int i = 0,n = 0,count = 0;
    cdfinger_spi_data data;
    unsigned char tx[12] = {0};
    unsigned char rx[12] = {0};

    data.tx = tx;
    data.rx = rx;

    memset(data.tx,0x00,12);
    data.tx[0] = 0x06;
    data.length = 1;
    data.tx_length = 1;
    spi_send_data(&data);

    usleep(8);
    check_busy();

    count = sizeof(MIPI_TO_SPI_IMPL_V18_NVCM)/12;
    for(i=0;i<count;++i)
    {
        int j = 0;
        ++n;
        memset(data.tx,0,12);
        memcpy(data.tx,MIPI_TO_SPI_IMPL_V18_NVCM+i*12,12);
        for(j=0;j<12;++j){
            printf("%02x  ",data.tx[j]);
        }
        printf("[%d]\n",n);
        data.length = 12;
        data.tx_length = 12;
        spi_send_data(&data);

        usleep(4000);
        check_busy();
    }
    printf("count ======== %d\n",count);

    memset(data.tx,0,12);
    data.tx[0] = 0x04;
    data.length = 1;
    data.tx_length = 1;
    spi_send_data(&data);

    usleep(8);
    
}

void VerifyNVCMMainArrays(void)
{
    int i = 0,n = 0,count = 0;
    cdfinger_spi_data data;
    unsigned char tx[21] = {0};
    unsigned char rx[21] = {0};

    data.tx = tx;
    data.rx = rx;

    memset(data.tx,0,21);
    data.tx[0] = 0x83;
    data.tx[3] = 0x25;
    data.length = 5;
    data.tx_length = 5;
    spi_send_data(&data);

    usleep(8);

    count = sizeof(MIPI_TO_SPI_IMPL_V18_NVCM)/12;
    for(i=0;i<count;++i)
    {
        memset(data.tx,0,21);
        data.tx[0] = 0x03;
        data.tx[1] = MIPI_TO_SPI_IMPL_V18_NVCM[i*12+1];
        data.tx[2] = MIPI_TO_SPI_IMPL_V18_NVCM[i*12+2];
        data.tx[3] = MIPI_TO_SPI_IMPL_V18_NVCM[i*12+3];
        data.length = 21;
        data.tx_length = 21;
        spi_send_data(&data);
        
        int j = 0;
        for (j = 0; j < 8; ++j)
        {
            if(data.rx[j] != MIPI_TO_SPI_IMPL_V18_NVCM[i*12+4+j]){
                printf("verify error, data.tx[%d]=0x%02x, MIPI_TO_SPI_IMPL_V18_NVCM[%d]=0x%02x\n", \
                    j,data.rx[j],i*12+4+j,MIPI_TO_SPI_IMPL_V18_NVCM[i*12+4+j]);
                exit(1);
            }
        }
        

        usleep(8);
    }
}

void programming_NVCM(void)
{
#if 0
    /* Power up & Connect Programmer to NVCM */
    PowerUpAndConnectNVCM(0);
    
    /* Set up Reading Parameter in Trim Register */
    SetupReadingParameter();

    /* Verify Chip ID */
    VerifyChipID();
   
    /* Blank Check on NVCM Trim_Parameter_OTP Block */
    BlankCheckOnNVCM_OTP_Block();
    
    /* Blank Check on NVCM Main Memory Block */
    BlankCheckOnNVCMMainMemoryBlock();

    /* Setup Programming Parameter in Trim Registers */
    SetupProgrammingParameter();

    /* Program NVCM Main Mermory in Variable Time Mode */
    ProgramNVCMMainMermory();

    /* Verify the NVCM main Arrays */
    VerifyNVCMMainArrays();
#endif

#if 1
    int i = 0,n = 0,count = 0;
    cdfinger_spi_data data;
    data.tx = (unsigned char *)malloc(1024);
    data.rx = (unsigned char *)malloc(1024);
    memset(data.tx,0x00,1024);
    memset(data.rx,0x00,1024);

    // ioctl(m_fd,CDFINGER_CONTROL_CS,0);
    // ioctl(m_fd,CDFINGER_HW_RESET,0);
    // // ioctl(m_fd,CDFINGER_CONTROL_CLK,0);
    // usleep(130);
    // // ioctl(m_fd,CDFINGER_CONTROL_CLK,1);
    // ioctl(m_fd,CDFINGER_CONTROL_CS,1);
    // ioctl(m_fd,CDFINGER_HW_RESET,1);
    // usleep(1000*300);
    // ioctl(m_fd,CDFINGER_CONTROL_CS,0);
    // ioctl(m_fd,CDFINGER_HW_RESET,0);
    // usleep(130);
    // ioctl(m_fd,CDFINGER_CONTROL_CS,1);
    // ioctl(m_fd,CDFINGER_HW_RESET,1);

    // usleep(1000*300);

    ioctl(m_fd,CDFINGER_CONTROL_CS,0);
    usleep(1000*102);
    ioctl(m_fd,CDFINGER_HW_RESET,0);
    usleep(1000*104);
    ioctl(m_fd,CDFINGER_CONTROL_CS,1);
    usleep(195);
    ioctl(m_fd,CDFINGER_CONTROL_CS,0);
    usleep(1000*101);
    ioctl(m_fd,CDFINGER_HW_RESET,1);

    usleep(1000*300);

    ioctl(m_fd,CDFINGER_INIT_GPIO);
    ioctl(m_fd,CDFINGER_SPI_MODE,0);

    memset(data.tx,0,6);
    data.tx[0] = 0x7e;
    data.tx[1] = 0xaa;
    data.tx[2] = 0x99;
    data.tx[3] = 0x7e;
    data.tx[4] = 0x01;
    data.tx[5] = 0x0e;
    data.length = 6;
    data.tx_length = 6;
    spi_send_data(&data);
    
    for ( i = 0; i < 6; i++)
    {
        printf("0x%02x\n",data.rx[i]);
    } 

    printf("\n\n\n");

    check_busy();

    memset(data.tx,0,12);
    data.tx[0] = 0x82;
    data.tx[1] = 0x00;
    data.tx[2] = 0x00;
    data.tx[3] = 0x20;
    data.tx[4] = 0x00;
    data.tx[5] = 0x15;
    data.tx[6] = 0xf2;
    data.tx[7] = 0xf0;
    data.tx[8] = 0xa2;
    data.tx[9] = 0x00;
    data.tx[10] = 0x00;
    data.tx[11] = 0x00;
    data.length = 12;
    data.tx_length = 12;
    spi_send_data(&data);

    for ( i = 0; i < 12; i++)
    {
        printf("0x%02x\n",data.rx[i]);
    } 
    printf("\n\n\n");

    check_busy();

    memset(data.tx,0,5);
    data.tx[0] = 0x83;
    data.tx[1] = 0x00;
    data.tx[2] = 0x00;
    data.tx[3] = 0x25;
    data.tx[4] = 0x20;
    data.length = 5;
    data.tx_length = 5;
    spi_send_data(&data);
    for ( i = 0; i < 5; i++)
    {
        printf("0x%02x\n",data.rx[i]);
    } 
    printf("\n\n\n");

    check_busy();

    memset(data.tx,0,14);
    data.tx[0] = 0x03;
    data.length = 40;
    data.tx_length = 40;
    spi_send_data(&data);
    for ( i = 0; i < 14; i++)
    {
        printf("0x%02x\n",data.rx[i]);
    } 
    printf("\n\n\n");

    check_busy();

    // check_busy();

    // memset(data.tx,0,5);
    // data.tx[0] = 0x83;
    // data.tx[3] = 0x25;
    // data.tx[4] = 0x00;
    // data.length = 5;
    // data.tx_length = 5;
    // spi_send_data(&data);

    // check_busy();

    // memset(data.tx,0,5);
    // data.tx[0] = 0x83;
    // data.tx[3] = 0x25;
    // data.tx[4] = 0x00;
    // data.length = 5;
    // data.tx_length = 5;
    // spi_send_data(&data);

    // check_busy();

    // memset(data.tx,0,2);
    // data.tx[0] = 0x05;
    // data.tx[1] = 0x00;
    // data.length = 2;
    // data.tx_length = 2;
    // spi_send_data(&data);
    // printf("data.rx[0]=0x%02x, data.rx[1]=0x%02x\n",data.rx[0],data.rx[1]);
#endif
    
    // memset(data.tx,0,SIZE_T);
    // data.tx[0] = 0x83;
    // data.tx[3] = 0x25;
    // data.length = 5;
    // data.tx_length = 5;
    // spi_send_data(&data);

    // usleep(8);

    // memset(data.tx,0,SIZE_T);
    // data.tx[0] = 0x83;
    // data.tx[3] = 0x25;
    // data.length = 5;
    // data.tx_length = 5;
    // spi_send_data(&data);

    // usleep(8);
    // check_busy(&data);

    // memset(data.tx,0,SIZE_T);
    // data.tx[0] = 0x06;
    // data.length = 1;
    // data.tx_length = 1;
    // spi_send_data(&data);

    // usleep(8);
    // check_busy(&data);

    // memset(data.tx,0,SIZE_T);
    // data.tx[0] = 0x02;
    // data.tx[3] = 0x20;
    // data.tx[5] = 0x15;
    // data.tx[6] = 0xf2;
    // data.tx[7] = 0xf1;
    // data.tx[8] = 0xc4;
    // data.length = 12;
    // data.tx_length = 12;
    // spi_send_data(&data);

    // usleep(8);
    // check_busy(&data);

    // memset(data.tx,0,SIZE_T);
    // data.tx[0] = 0x02;
    // data.tx[3] = 0x60;
    // data.tx[5] = 0x15;
    // data.tx[6] = 0xf2;
    // data.tx[7] = 0xf1;
    // data.tx[8] = 0xc4;
    // data.length = 12;
    // data.tx_length = 12;
    // spi_send_data(&data);

    // usleep(8);
    // check_busy(&data);

    // memset(data.tx,0,SIZE_T);
    // data.tx[0] = 0x02;
    // data.tx[3] = 0xa0;
    // data.tx[5] = 0x15;
    // data.tx[6] = 0xf2;
    // data.tx[7] = 0xf1;
    // data.tx[8] = 0xc4;
    // data.length = 12;
    // data.tx_length = 12;
    // spi_send_data(&data);

    // usleep(8);
    // check_busy(&data);

    // memset(data.tx,0,SIZE_T);
    // data.tx[0] = 0x02;
    // data.tx[3] = 0xe0;
    // data.tx[5] = 0x15;
    // data.tx[6] = 0xf2;
    // data.tx[7] = 0xf1;
    // data.tx[8] = 0xc4;
    // data.length = 12;
    // data.tx_length = 12;
    // spi_send_data(&data);

    // usleep(8);
    // check_busy(&data);

    // memset(data.tx,0,SIZE_T);
    // data.tx[0] = 0x03;
    // data.tx[3] = 0x20;
    // data.length = 21;
    // data.tx_length = 21;
    // spi_send_data(&data);

    // usleep(8);

    // memset(data.tx,0,SIZE_T);
    // data.tx[0] = 0x04;
    // data.length = 1;
    // data.tx_length = 1;
    // spi_send_data(&data);

    // usleep(8);

    // memset(data.tx,0,SIZE_T);
    // data.tx[0] = 0x83;
    // data.tx[3] = 0x25;
    // data.length = 5;
    // data.tx_length = 5;
    // spi_send_data(&data);

    // usleep(8);
    // check_busy(&data);

    // ioctl(m_fd,CDFINGER_CONTROL_RESET,0);
    // usleep(200);
    // ioctl(m_fd,CDFINGER_CONTROL_RESET,1);

    // /* check for NVCM Programming Success */




    // /* Perform and Security Bit Programming */
    // memset(data.tx,0,SIZE_T);
    // data.tx[0] = 0x83;
    // data.tx[3] = 0x25;
    // data.length = 5;
    // data.tx_length = 5;
    // spi_send_data(&data);

    // usleep(8);

    // memset(data.tx,0,SIZE_T);
    // data.tx[0] = 0x83;
    // data.tx[3] = 0x25;
    // data.tx[4] = 0x10;
    // data.length = 5;
    // data.tx_length = 5;
    // spi_send_data(&data);

    // usleep(8);
    // check_busy(&data);

    // memset(data.tx,0,SIZE_T);
    // data.tx[0] = 0x06;
    // data.length = 1;
    // data.tx_length = 1;
    // spi_send_data(&data);

    // usleep(8);
    // check_busy(&data);

    // memset(data.tx,0,SIZE_T);
    // data.tx[0] = 0x02;
    // data.tx[3] = 0x20;
    // data.tx[4] = 0x30;
    // data.tx[7] = 0x01;
    // data.length = 12;
    // data.tx_length = 12;
    // spi_send_data(&data);

    // usleep(4000);
    // check_busy(&data);

    // memset(data.tx,0,SIZE_T);
    // data.tx[0] = 0x02;
    // data.tx[3] = 0x60;
    // data.tx[4] = 0x30;
    // data.tx[7] = 0x01;
    // data.length = 12;
    // data.tx_length = 12;
    // spi_send_data(&data);

    // usleep(4000);
    // check_busy(&data);

    // memset(data.tx,0,SIZE_T);
    // data.tx[0] = 0x02;
    // data.tx[3] = 0xa0;
    // data.tx[4] = 0x30;
    // data.tx[7] = 0x01;
    // data.length = 12;
    // data.tx_length = 12;
    // spi_send_data(&data);

    // usleep(4000);
    // check_busy(&data);

    // memset(data.tx,0,SIZE_T);
    // data.tx[0] = 0x02;
    // data.tx[3] = 0xe0;
    // data.tx[4] = 0x30;
    // data.tx[7] = 0x01;
    // data.length = 12;
    // data.tx_length = 12;
    // spi_send_data(&data);

    // usleep(4000);
    // check_busy(&data);

    // memset(data.tx,0,SIZE_T);
    // data.tx[0] = 0x03;
    // data.tx[3] = 0x20;
    // data.length = 21;
    // data.tx_length = 21;
    // spi_send_data(&data);

    // usleep(8);

    // memset(data.tx,0,SIZE_T);
    // data.tx[0] = 0x83;
    // data.tx[3] = 0x25;
    // data.length = 5;
    // data.tx_length = 5;
    // spi_send_data(&data);

    // usleep(8);
    // check_busy(&data);

out:
    printf("programming is success!!!\n");
    
    return;
}

int selectP2()
{
    uint8_t tx[8] = {0};
    uint8_t rx[8] = {0};

    memset(tx, 0x66, 8);
    memset(rx, 0x66, 8);
    tx[0] = 0x8d;
    tx[1] = 0x80;
    tx[2] = 0x66;
    tx[3] = 0x66;
    spi_send_data_m(tx,rx,4);
    for (int i = 0; i < 8; i++)
    {
        printf("rx[%d]=%x ", i, rx[i]);
    }
    printf("\n");

    if (rx[4] != 0x20)
    {
        printf("==============downloader program fail=================\n");
    }

    return 0;
}

void cpld_test(void)
{
   int i = 0, length = 0, ret = -1;
    unsigned char *tx = NULL;
    unsigned char *rx = NULL;

    FILE *fp = fopen("/data/cpld.bin", "rb+");
    if (fp == NULL)
    {
        printf("open file error \n");
        goto out;
    }
    fseek(fp, 0L, SEEK_END);
    length = ftell(fp);
    printf("bin file length = %d \n", length);
    fseek(fp, 0L, SEEK_SET);
    tx = (unsigned char *)malloc(length);
    if (tx == NULL)
    {
        printf("malloc failed,data.tx==FULL!!!\n");
        goto out;
    }
    rx = (unsigned char *)malloc(length);
    if (rx == NULL)
    {
        printf("malloc failed,data.rx==FULL!!!\n");
        goto out;
    }

    // /ioctl(m_fd, CDFINGER_HW_RESET, 0);
    selectP2();
    ioctl(m_fd, CDFINGER_INIT_GPIO, 0);
    ioctl(m_fd, CDFINGER_CONTROL_CS, 0);
    ioctl(m_fd, CDFINGER_CONTROL_CLK, 1);
    usleep(1000);
    // ioctl(m_fd, CDFINGER_HW_RESET, 1);
    usleep(1000);
    ioctl(m_fd, CDFINGER_CONTROL_CS, 1);

    send_clk(8);
    
    ioctl(m_fd, CDFINGER_INIT_GPIO, 1);
    ioctl(m_fd, CDFINGER_SPI_MODE, 3);

    ret = fread(tx, sizeof(uint8_t), length, fp);
    if (ret != length)
    {
        printf("fread error!!!!!!!!!!!!!!!!!!!!!!!\n");
        ret = -1;
        goto out;
    }

    if (spi_send_data_m(tx, rx, length) < 0)
    {
        ret = -1;
        goto out;
    }

    ioctl(m_fd, CDFINGER_INIT_GPIO, 0);
    send_clk(200);

    ioctl(m_fd, CDFINGER_INIT_GPIO, 1);
    ioctl(m_fd, CDFINGER_SPI_MODE, 0);

    ret = 0;
out:
    if (tx != NULL)
        free(tx);
    if (rx != NULL)
        free(rx);
    if (fp != NULL)
        fclose(fp);
    return;

}