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


void cmos_init(void){
    /*read id*/
    printf("camera 768 x 576 init start\n");
    cdfinger_spi_data data;
    uint8_t tx[10] = {0};
    uint8_t rx[10] = {0};
    uint8_t data_f8 = 0,data_02 = 0;

    data.tx = tx;
    data.rx = rx;

    data.tx[0] = 0x8d;
    data.tx[1] = 0x00;
    data.tx[2] = 0x66;
    data.tx[3] = 0x66;
    data.length = 4;
    data.tx_length = 4;
    spi_send_data(&data);

    cdfinger_camera_read_id();

    /*set camera*/
//////////////////////   IRQ enable   //////////////////////
    data.tx[0] = 0x88;
    data.tx[1] = 0x66;
    data.tx[2] = 0x66;
    data.tx[3] = 0x66;
    data.length = 4;
    data.tx_length = 4;
    spi_send_data(&data);

//////////////////////   IRQ clear   //////////////////////
    data.tx[0] = 0x89;
    data.tx[1] = 0x66;
    data.tx[2] = 0x66;
    data.length = 3;
    data.tx_length = 3;
    spi_send_data(&data);
//////////////////////   SYS   //////////////////////
#if 1
printf("please set 0xf8 data:");
scanf("%d",&data_f8);
printf("please set 0x02 data:");
scanf("%d",&data_02);
i2c_write_data(0xfe,0x80);
i2c_write_data(0xfe,0x80);
i2c_write_data(0xfe,0x80);
i2c_write_data(0xf2,0x00);//sync_pad_io_ebi
i2c_write_data(0xf6,0x00);//up down
i2c_write_data(0xfc,0xc6);
i2c_write_data(0xf7,0x19);//pll enable
i2c_write_data(0xf8,data_f8);//Pll mode 2
i2c_write_data(0xf9,0x3e);//[0] pll enable//解决IOVDD 电流大问题
i2c_write_data(0xfe,0x03);
i2c_write_data(0x06,0x80);
i2c_write_data(0x06,0x00);
i2c_write_data(0xfe,0x00);
i2c_write_data(0xf9,0x2e);
i2c_write_data(0xfe,0x00);
i2c_write_data(0xfa,0x00);//div
i2c_write_data(0xfe,0x00);
////////////////   ANALOG & CISCTL   ////////////////
i2c_write_data(0x03,0x02);
i2c_write_data(0x04,0x55);
i2c_write_data(0x05,0x00);
i2c_write_data(0x06,0xbb);
i2c_write_data(0x07,0x00);
i2c_write_data(0x08,0x46);
i2c_write_data(0x0c,0x04);
i2c_write_data(0x0d,0x02);
i2c_write_data(0x0e,0x48);
i2c_write_data(0x0f,0x03);
i2c_write_data(0x10,0x08);
i2c_write_data(0x11,0x23);//44FPN 列异常
i2c_write_data(0x12,0x10);
i2c_write_data(0x13,0x11);
i2c_write_data(0x14,0x01);
i2c_write_data(0x15,0x00);
i2c_write_data(0x16,0xc0);
i2c_write_data(0x17,0x14);
i2c_write_data(0x18,0x02);
i2c_write_data(0x19,0x38);
i2c_write_data(0x1a,0x11);
i2c_write_data(0x1b,0x06);
i2c_write_data(0x1c,0x04);
i2c_write_data(0x1d,0x00);
i2c_write_data(0x1e,0xfc);
i2c_write_data(0x1f,0x09);
i2c_write_data(0x20,0xb5);
i2c_write_data(0x21,0x3f);
i2c_write_data(0x22,0xe6);
i2c_write_data(0x23,0x32);
i2c_write_data(0x24,0x2f);
i2c_write_data(0x27,0x00);
i2c_write_data(0x28,0x00);
i2c_write_data(0x2a,0x00);
i2c_write_data(0x2b,0x03);
i2c_write_data(0x2c,0x00);
i2c_write_data(0x2d,0x01);
i2c_write_data(0x2e,0xf0);
i2c_write_data(0x2f,0x01);
i2c_write_data(0x25,0xc0);
i2c_write_data(0x3d,0xe0);
i2c_write_data(0x3e,0x45);
i2c_write_data(0x3f,0x1f);
i2c_write_data(0xc2,0x17);
i2c_write_data(0x30,0x22);
i2c_write_data(0x31,0x23);
i2c_write_data(0x32,0x02);
i2c_write_data(0x33,0x03);
i2c_write_data(0x34,0x04);
i2c_write_data(0x35,0x05);
i2c_write_data(0x36,0x06);
i2c_write_data(0x37,0x07);
i2c_write_data(0x38,0x0f);
i2c_write_data(0x39,0x17);
i2c_write_data(0x3a,0x1f);
//////////////////////   ISP   //////////////////////
i2c_write_data(0xfe,0x00);
i2c_write_data(0x8a,0x00);
i2c_write_data(0x8c,0x07);//07
i2c_write_data(0x8d,0x03);//0xb8-128  0x38-192
i2c_write_data(0x8b,0xa2);
i2c_write_data(0x8e,0x02);//luma value not normal
i2c_write_data(0x90,0x01);
i2c_write_data(0x94,0x02);
i2c_write_data(0x95,0x02);
i2c_write_data(0x96,0x40);
i2c_write_data(0x97,0x03);
i2c_write_data(0x98,0x00);
//////////////////////	 BLK	/////////////////////
i2c_write_data(0xfe,0x00);
i2c_write_data(0x18,0x02);
i2c_write_data(0x40,0x22);
i2c_write_data(0x41,0x01);
i2c_write_data(0x5e,0x00);
i2c_write_data(0x66,0x20);
//////////////////////	 MIPI	/////////////////////
i2c_write_data(0xfe,0x03);
i2c_write_data(0x01,0x83);
i2c_write_data(0x02,data_02);
i2c_write_data(0x03,0x96);
i2c_write_data(0x04,0x01);
i2c_write_data(0x05,0x00);
i2c_write_data(0x06,0xa4);
i2c_write_data(0x10,0x90);
i2c_write_data(0x11,0x2b);
i2c_write_data(0x12,0xc0);
i2c_write_data(0x13,0x03);
i2c_write_data(0x15,0x02);
i2c_write_data(0x21,0x10);
i2c_write_data(0x22,0x03);
i2c_write_data(0x23,0x20);
i2c_write_data(0x24,0x02);
i2c_write_data(0x25,0x10);
i2c_write_data(0x26,0x05);
i2c_write_data(0x21,0x10);
i2c_write_data(0x29,0x03);
i2c_write_data(0x2a,0x0a);
i2c_write_data(0x2b,0x04);
i2c_write_data(0xfe,0x00);
i2c_write_data(0xb0,0x50);
i2c_write_data(0xb6,0x00);
printf("camera init done\n\n\n");
#endif

    data.tx[0] = 0x88;
    data.tx[1] = 0x66;
    data.tx[2] = 0x66;
    data.tx[3] = 0x66;
    data.length = 4;
    data.tx_length = 4;
    spi_send_data(&data);
    //sleep(10);
    //get_pictures();
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

int get_pictures(void)
{
    int semcnts = 0 ,error = 0;
    long long t0 = 0;
    long long t1 = 0,t2 = 500;
    static long long SUM_time = 0;
#ifdef PRINTF_ERROR_PIXEL
    struct img_error_data *error_img;
#endif
    uint32_t p_count = 0,img_num = 0, cpld_rst = 0, img_error = 0,pixel_error = 0;
    cdfinger_spi_data data;
    int ret = -1, i = 0, j = 0;
    uint16_t *img_data = NULL;

    printf("CMOS_BUFFER_SIZE = %d\n",CMOS_BUFFER_SIZE);
    printf("how many pictures do you want:\n");
    scanf("%d",&p_count);
    img_data = (uint16_t *)malloc(sizeof(uint16_t)*CMOS_BUFFER_SIZE);
    data.tx = (uint8_t *)malloc(sizeof(uint8_t)*CMOS_BUFFER_SIZE);
    data.rx = (uint8_t *)malloc(sizeof(uint8_t)*CMOS_BUFFER_SIZE);
#ifdef PRINTF_ERROR_PIXEL
    error_img = malloc(sizeof(struct img_error_data)*p_count*384*258);//打印错误像素点所属帧号,以及错误像素点位于一帧中哪个像素点以及像素点的值
　　 memset(error_img,0,sizeof(struct img_error_data)*p_count*384*258);
#endif
    while(p_count--){
            //sleep(2);
    //printf("pictures p_count=%d:\n",p_count);

    //printf("send 0x8d/02 for standby \n");
    data.tx[0] = 0x8d;
    data.tx[1] = 0x02;
    data.tx[2] = 0x66;
    data.tx[3] = 0x66;
    data.length = 4;
    data.tx_length = 4;
    spi_send_data(&data);
    //printf("0x8d:0x%02x  0x%02x  0x%02x  0x%02x \n",data.rx[0],data.rx[1],data.rx[2],data.rx[3]);

    //printf("send 0x8d/40  wait interrupt \n");
    data.tx[0] = 0x8d;
    data.tx[1] = 0x40;
    data.tx[2] = 0x66;
    data.tx[3] = 0x66;
    data.length = 4;
    data.tx_length = 4;
    spi_send_data(&data);
    //printf("0x8d:0x%02x  0x%02x  0x%02x  0x%02x \n",data.rx[0],data.rx[1],data.rx[2],data.rx[3]);

        memset(data.tx, 0x66, (CMOS_BUFFER_SIZE));
        memset(data.rx, 0x00, (CMOS_BUFFER_SIZE));
        data.tx[0]     = 0x8a;
        data.length    = CMOS_BUFFER_SIZE;
        data.tx_length = CMOS_BUFFER_SIZE;

        ++img_num;

        t0 = cfp_get_uptime(); 
#if 1
        ret = WaitSemCall();
        if(ret == -1){
            //return -1;
            printf("send cpld cmd make cpld reset to resurrect!!\n");
            cpld_rst++;
            data.tx[0] = 0xf0;
            data.tx[1] = 0x66;
            data.tx[2] = 0x66;
            data.tx[3] = 0x66;
            data.length = 4;
            data.tx_length = 4;
            spi_send_data(&data);
            break;
        }
#endif
        spi_send_data(&data);
        t0 = cfp_get_uptime() - t0;
        SUM_time += t0;
        if(t0>t1)
            t1 = t0;
        if(t0 < t2)
            t2 = t0;
        printf("Capturing the %dth picture takes %lld ms,average time=%lld, MAX time = %lld ms,MIN time = %lld ms\n",img_num,t0,SUM_time/img_num,t1,t2);

        //printf("0x8d:0x%02x  0x%02x  0x%02x  0x%02x \n",data.rx[0],data.rx[1],data.rx[2],data.rx[3]);
        // j = 0;
        // for(i=3;i<CMOS_BUFFER_SIZE;i+=3){
        //    img_data[j] = (data.rx[i] | (data.rx[i+1]<<8))&0x0fff;
        //    img_data[j+1] = ((data.rx[i+1]>>4) | (data.rx[i+2]<<4))&0x0fff;
        //    j += 2;
        // }
        //printf("Capturing a picture takes %d ms\n",cfp_get_uptime() - t0);

        // for(i=0;i<100;++i){
        //     printf("data.rx[%d]=0x%02x\n",i,data.rx[i]);
        // }

        j = 0;
        for(i=3;i<CMOS_BUFFER_SIZE;i+=5){
           img_data[j] = (data.rx[i] | (data.rx[i+1]<<8))&0x03ff;
           img_data[j+1] = ((data.rx[i+1]>>2) | (data.rx[i+2]<<6))&0x03ff;
           img_data[j+2] = ((data.rx[i+2]>>4) | (data.rx[i+3]<<4))&0x03ff;
           img_data[j+3] = ((data.rx[i+3]>>6) | (data.rx[i+4]<<2))&0x03ff;
           j += 4;
        }

        //printf("send 0x8d/02 for standby \n");
        data.tx[0] = 0x8d;
        data.tx[1] = 0x02;
        data.tx[2] = 0x66;
        data.tx[3] = 0x66;
        data.length = 4;
        data.tx_length = 4;
        spi_send_data(&data);
        //printf("j=%d\n",j);
        // for(i=0;i<224;i++)
        //     printf("data.rx[%d]=0x%02x    ",i,data.rx[i]);
        // printf("\n");

        // j = 0;
        // for(i=0;i<CMOS_BUFFER_SIZE;i+=5){
        //     m_bit_p = (struct m_bit *)&data.rx[i];
        //    img_data[j] = m_bit_p->a;
        //    img_data[j+1] = m_bit_p->b;
        //    img_data[j+2] = m_bit_p->c;
        //    img_data[j+3] = m_bit_p->d;
        //    j += 4;
        // }
        for(i=0;i<WIDTH*HIGHT;i++){
            data.rx[i] = (uint8_t)(img_data[i]>>2);
            //if(i < 10)
            //printf("img[%d]=0x%04x\n",i,img_data[i]);//1100 0000 0000
            if(data.rx[i]!= 0x80){
                error = 1;
 #ifdef PRINTF_ERROR_PIXEL
                error_img[pixel_error].img_num = img_num;
                error_img[pixel_error].pixel_num = i;
                error_img[pixel_error].data_rx = data.rx[i];
                error_img[pixel_error].img_data = img_data[i];
 #endif
               printf("compare with 0xc0,the %dth img data error!!data.rx[%d]=0x%04x!!img_data[%d]=0x%04x\n",img_num,i,data.rx[i],i,img_data[i]);
                pixel_error++;
                //exit(1);
            }
        }
        if(1 == error){
            img_error++;
            error = 0;
        }
        printf("pixel_error====%d\n",pixel_error);
        draw_image(WIDTH,HIGHT,data.rx,NULL);
        usleep(100);
        memset(data.rx, 0x00, (CMOS_BUFFER_SIZE));
        // draw_image(WIDTH,HIGHT,data.rx,NULL);
    }
        printf("\ntotal test %d imgs ,have %d imgs error,%d pixels error,cpld reset %d times\n",img_num,img_error,pixel_error,cpld_rst);
 #ifdef PRINTF_ERROR_PIXEL
        for(i = 0; i < pixel_error ;i++)
        {
            printf("compare with 0xc0,the %dth img data error!!data.rx[%d]=0x%04x!!img_data[%d]=0x%04x\n",
                error_img[i].img_num,
                error_img[i].pixel_num,error_img[i].data_rx,
                error_img[i].pixel_num,error_img[i].img_data);
        }
#endif
        free(data.rx);
        free(data.tx);
        SUM_time = 0;
    return 0;
}


// int get_pictures(void){
//     /*test full buffer mode*/
//     long i = 0;
//     long j = 0;
//     long x = 0;
//     long y = 0;
//     long z = 0;
//     long pixel_count = 0;
//     int ret = 0;
//     int num_of_aa = 0;
//     int k = 0;
//     long long t0 = 0;
//     long number_of_default = 0;
//     int t_num = 0;
//     uint16_t pic_data_12[224*224];
//     uint8_t pic_data_8[224*224];
//     cdfinger_spi_data *data = malloc(sizeof(cdfinger_spi_data));
//     uint8_t *cdfinger_pic_data = malloc(sizeof(unsigned char)*CMOS_BUFFER_SIZE);
//     data->tx = malloc(sizeof(unsigned char)*CMOS_BUFFER_SIZE);
//     if(data == NULL || data->tx == NULL || cdfinger_pic_data == NULL){
//         printf("malloc default=====>cmos.c\n");
//         return -1;
//     }
//     data->rx = cdfinger_pic_data;

// retry:
//     printf("how many pictures do you want(enter 999 go back):");
//     scanf("%d",&i);
//     printf("\n");
//     number_of_default=0;

//     int fg = 0;
//     if(fg){
//         while(i > 0){
//             i--;
//             j++;
//             printf("picture %ld~\n",j);
            
//             ioctl(m_fd,CDFINGER_HW_RESET);
        
//             memset(data->tx, 0x66, (CMOS_BUFFER_SIZE));
//             memset(data->rx, 0x00, (CMOS_BUFFER_SIZE));
//             data->tx[0]     = 0x8a;
//             data->length    = CMOS_BUFFER_SIZE;
//             data->tx_length = CMOS_BUFFER_SIZE;  

//             t0 = cfp_get_uptime(); 
//             cdfinger_camera_capture();
//             usleep(27*1000);             
//             spi_send_data(data);
//             printf("Capturing a picture takes %d ms\n",cfp_get_uptime() - t0);

//             x = 0;
//             z = 0;
//             t_num = 0;
//             memset(pic_data_8,0x00,224*224);
//             for(num_of_aa = 0; num_of_aa < 224&&x<CMOS_BUFFER_SIZE-336;){            
//                 if(cdfinger_pic_data[x] == 0xaa){
//                     num_of_aa++;
//                     for(y = x+1; y < x+337; y += 3){
//                         pic_data_12[z] = ((data->rx[y+1]<<8)&0x0f00) | ((data->rx[y])&0x00ff);
//                         pic_data_8[z] = (uint8_t)(pic_data_12[z]>>4);
//                         z++;
//                         pic_data_12[z] = ((data->rx[y+1]>>4)&0x000f) | (((data->rx[y+2])<<4)&0x0ff0);
//                         pic_data_8[z] = (uint8_t)(pic_data_12[z]>>4);
//                         z++;
//                     }
//                     x+=336;     
//                 }else{
//                     x++;
//                 }
//             }        
//             printf("\n");
//             printf("num_of_aa = %d\n",num_of_aa);
//             if(num_of_aa!=224){
//                 number_of_default++;
//                 printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
//                 //exit(1);
//                 //goto retry;
//                 // if(number_of_default==9)
//                 //    break;
//             }

//             // int my_i;
//             // for(my_i=0;my_i<224*224;my_i++){
//             //     if(pic_data_12[my_i]!=dig){
//             //         t_num++;
//             //         printf("dig ================= 0x%04x\n",dig);
//             //         printf("n=%d    data[%d]=0x%04x\n",t_num,my_i,pic_data_12[my_i]);
//             //     }
//             // }
//             // if(t_num!=0)
//             //     exit(1);
//             draw_image(HIGHT,WIDTH,pic_data_8,NULL);
//             printf("number of default = %ld\n",number_of_default);

//             printf("dig ================= 0x%04x\n",dig);
//         }
//     }else{
//         while(i > 0){
//             i--;
//             j++;
//             printf("picture %ld~\n",j);
            
//             ioctl(m_fd,CDFINGER_HW_RESET);

//             data->rx = cdfinger_pic_data;
//             memset(data->tx, 0x66, (CMOS_BUFFER_SIZE));
//             memset(data->rx, 0x00, (CMOS_BUFFER_SIZE));
//             data->tx[0]     = 0x8a;
//             data->length    = CMOS_BUFFER_SIZE;
//             data->tx_length = CMOS_BUFFER_SIZE;  

//             t0 = cfp_get_uptime(); 
//             //cdfinger_camera_capture();
//             ret = WaitSemCall();
//             if(ret == -1){
//                 return -1;
//             }

//             memset(data->rx,0x00,CMOS_BUFFER_SIZE);
//             //ret = read(m_fd,data->rx,CMOS_BUFFER_SIZE);
//             spi_send_data(data);
        
//             printf("Capturing a picture takes %d ms\n",cfp_get_uptime() - t0);
        
//             x = 0;
//             z = 0;
//             t_num = 0;
//             memset(pic_data_8,0x00,sizeof(pic_data_8));
//             memset(pic_data_12,0x00,sizeof(pic_data_12));
//             for(num_of_aa = 0; num_of_aa < 224&&x<CMOS_BUFFER_SIZE-336;){            
//                 if(cdfinger_pic_data[x] == 0xaa){
//                 // printf("0x%02x  0x%02x  0x%02x  0x%02x\n",cdfinger_pic_data[x-3],cdfinger_pic_data[x-2],cdfinger_pic_data[x-1],cdfinger_pic_data[x]);
//                     num_of_aa++;
//                     for(y = x+1; y < x+337; y += 3){
//                         pic_data_12[z] = ((data->rx[y+1]<<8)&0x0f00) | ((data->rx[y])&0x00ff);
//                         pic_data_8[z] = (uint8_t)(pic_data_12[z]>>4);
//                         z++;
//                         pic_data_12[z] = ((data->rx[y+1]>>4)&0x000f) | (((data->rx[y+2])<<4)&0x0ff0);
//                         pic_data_8[z] = (uint8_t)(pic_data_12[z]>>4);
//                         z++;
//                     }
//                     x+=336;           
//                 }else{
//                     x++; 
//                 }
//             }
//             printf("\n");
//             printf("num_of_aa = %d\n",num_of_aa);
//             if(num_of_aa!=224){
//                 number_of_default++;
//                 printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
//                 //exit(1);
//                 //goto retry;
//                 // if(number_of_default==9)
//                 break;
//             }
//             // for(i=0;i<224*224;i++){
//             //     if(pic_data_12[i]!=dig){
//             //         t_num++;
//             //         printf("dig ================= 0x%04x\n",dig);
//             //         printf("n=%d    data[%d]=0x%04x\n",t_num,i,pic_data_12[i]);
//             //     }
//             // }
//             // if(t_num!=0)
//             //      exit(1);
//             draw_image(HIGHT,WIDTH,pic_data_8,NULL);
//             printf("number of default = %ld\n",number_of_default);
//         }
//     }
//     free(data->tx);
//     free(cdfinger_pic_data);
//     free(data);
// }

void move_picture(void){
    
}

void change_exposure_time(void){

}

void set_reg(void)
{
    uint8_t reg = 0;
    uint8_t val = 0;
    printf("please input the regadress and value: \n");
    scanf("%x %x",&reg,&val);
    printf("reg=%02x,val=%02x\n",reg,val);
    i2c_write_data(reg,val);
    printf("read val = %02x\n",i2c_read_data(reg));
    return;
}

void config_flash(void){
    uint8_t w_data[1024*100] = {0};
    uint8_t r_data[1024*100] = {0};
    int i = 0;
    int len = 87250;
    for(i=0;i<len;i++)
        w_data[i] = i;

    

    page_program_size(0x10000, w_data, len);
    read_flash(0x03,0x10000,r_data, len);

    for(i=0;i<len;i++){
        if(r_data[i] != w_data[i]){
            printf("data error!!!!!!!!!!!!!!!!!!!!!!!!\n");
        }
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
    int i = 0;
    cdfinger_spi_data data;
    ioctl(m_fd,CDFINGER_CONTROL_RESET,0);
    ioctl(m_fd,CDFINGER_CONTROL_CS,0);
    ioctl(m_fd,CDFINGER_CONTROL_CLK,1);
    usleep(1000);
    ioctl(m_fd,CDFINGER_CONTROL_RESET,1);
    usleep(1000);
    ioctl(m_fd,CDFINGER_CONTROL_CS,1);
    ioctl(m_fd,CDFINGER_SPI_MODE,4);
    uint8_t t[1] = {0};
    uint8_t r[1] = {0};
    data.tx = t;
    data.rx = r;
    data.length = 1;
    data.tx_length = 1;
    spi_send_data(&data);

    ioctl(m_fd,CDFINGER_SPI_MODE,3);

    uint8_t *ptr = NULL,*re = NULL;
    int ret = 0;
    int num = 0;
    char cpld_path[128];
    sprintf(cpld_path,"%s/%s","/data",CPLD_FILE_NAME);
    printf("download cpld bin file from path=%s\n",cpld_path);
    FILE *fp=fopen(cpld_path,"rb+");
    if(fp == NULL){
        printf("open file error\n");
        exit(1);
    }

    fseek(fp,0L,SEEK_END);
    num = ftell(fp);
    printf("bin file name=%s ,num = %d\n",CPLD_FILE_NAME,num);
    fseek(fp,0L,SEEK_SET);
    ptr=(uint8_t *)malloc(sizeof(uint8_t)*num);
    re=(uint8_t *)malloc(sizeof(uint8_t)*num);
    ret = fread(ptr,sizeof(uint8_t),num,fp);
    if(ret != num){
        printf("fread error!!!!!!!!!!!!!!!!!!!!!!!\n");
        exit(1);
    }

    data.tx = ptr;
    data.rx = re;

    data.length = num;
    data.tx_length = num;
    spi_send_data(&data);

    ioctl(m_fd,CDFINGER_SPI_MODE,4);

    memset(ptr,0,num);
    data.length = 150;
    data.tx_length = 150;
    spi_send_data(&data);

    usleep(100*1000);
    ioctl(m_fd,CDFINGER_SPI_MODE,0);

    free(ptr);
    free(re);
    fclose(fp);
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

void send_sclk(int count)
{
    int i = 0;
    for(i=0;i<count;++i){
        ioctl(m_fd,CDFINGER_CONTROL_CLK,0);
        usleep(1);
        ioctl(m_fd,CDFINGER_CONTROL_CLK,1);
        usleep(1);
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


void cpld_test(void)
{
    uint32_t count = 0,i = 0;
    printf("how many counts do you want test cpld:");
    scanf("%d",&count);
    while(count--){
    i++;
    printf("**********************the %dth cpld test**********************\n",i);
    ProgrammingRawCode();
    cmos_init();
    sleep(3);
    }
    printf("cpld test success!!!!\n");
}