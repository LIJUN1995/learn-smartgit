#ifndef __FINGERTECH_FINGERPRINT_H
#define __FINGERTECH_FINGERPRINT_H


#define EXAMPLE_LPSPI_CLOCK_SOURCE_SELECT (1U)
/* Clock divider for master lpspi clock source */
//#define EXAMPLE_LPSPI_CLOCK_SOURCE_DIVIDER (7U)
#define EXAMPLE_LPSPI_CLOCK_SOURCE_DIVIDER (4U)
#define LPSPI_MASTER_CLK_FREQ (CLOCK_GetFreq(kCLOCK_Usb1PllPfd1Clk) / (EXAMPLE_LPSPI_CLOCK_SOURCE_DIVIDER + 1U))

#define IMG_HEIGHT 96
#define IMG_WIDTH  (96*2)
#define FINGERTECH_READIMGSIZE   (28*1024)
#define FINGERTECH_IMAGESIZE     (IMG_HEIGHT*IMG_WIDTH)


#define  FINGERTECH_RESETCMD    0x0c
#define  FINGERTECH_STARTCMD    0x18
#define  FINGERTECH_DUMMY       0x66 
#define  FINGERTECH_INTCMD      0x14
#define  FINGERTECH_SLEEPCMD    0x00 
#define  FINGERTECH_READCHIPID  0x74 

/************************************
 * 通讯协议 Nile River
 * byte1  byte2  byte3  byte3  byte4  byte5  byte6  byte7    byte8
 * 0x4E   0x52   cmd    ret1   par2    par3   par4  crc16_h  crc16_l
************************************/
#define FINGERTECH_CMDHEADH         0x4E 
#define FINGERTECH_CMDHEADL         0x52

#define FINGERTECH_CMDFORENROLL     0x01
#define FINGERTECH_CMDAUTH          0x02
#define FINGERTECH_CMDGETTEMPLATE   0x03
#define FINGERTECH_CMDDELTEMPLATE   0x04
#define FINGERTECH_CMDSAVETEMPLATE  0x05

/**************************事件标志组************************/
#define FINGERTECH_EVENTNONE         (1<<0)
#define FINGERTECH_EVENTBITFORENROLL (1<<1)
#define FINGERTECH_EVENTBITFORAUTH   (1<<2)

//返回结果时第一个为是否成功，第二个为需要带的参数，第三位保留

/**************************错误码************************/
#define  FINGERTECH_SUCCESS         0x00 
#define  FINGERTECH_READIDERR       0x01 
#define  FINGERTECH_CONFIGREGERR    0x02 
#define  FINGERTECH_READIMGERR      0x03
#define  FINGERTECH_READFLASHERR    0x04
#define  FINGERTECH_ERASFLSHERR     0x05
#define  FINGERTECH_WRITEFLSHERR    0x06
#define  FINGERTECH_TEMPLATEALREADY 0x07
#define  FINGERTECH_TEMPDELETEREADY 0x08
#define  FINGERTECH_LISTERR         0x09



#define  FINGERTECH_FAIL            0xFF


//保存校准数据
//如果校准过则iscalibrate为0x05
struct fingertech_calibrationdata {
    uint8_t iscalibrate;
    uint8_t workagc;
    uint8_t intagc;
    uint8_t* bkimg;
};

typedef struct fingertech_calibrationdata fingertech_calibrationdata_t; 


//指纹工作状态
typedef enum fingertech_workerstate_t
{
    STATE_NONE   = -1, //无操作
    STATE_IDIE   = 0,  //空闲模式
    STATE_ENROLL = 1,  //注册
    STATE_AUTHENTICATE = 2, //比对
    STATE_WAIT_FINGER_UP = 3, //手指抬起
    STATE_CANCEL = 4,         //取消
}fingertech_workerstate_t; 

typedef struct fingertech_resutlt
{
    uint8_t fingerruncmd; //执行的命令码
    uint8_t fingertechresutlt;  //是否成功
    uint8_t fingertechresutltpara1;//参数1
    uint8_t fingertechresutltpara2;//参数2
}fingertech_resutlt_t;


//设备结构体
typedef struct fingertech_fpdevice_t
{
    SemaphoreHandle_t lock; //互斥锁
    SemaphoreHandle_t intflag;//二值信号量，用于标记中断是否产生
    SemaphoreHandle_t taskmutex;//任务二值互斥锁
    //TaskHandle_t thread;  //任务句柄
    TaskHandle_t fpenenroltaskhandle; //注册任务句柄
    TaskHandle_t fpauthtaskhandle;//比对任务句柄
    EventGroupHandle_t fingertecheventbit;//事件标志组
    QueueHandle_t fingertechqueue;//消息队类用于返回结果
    uint8_t  templateupdateflag;//模板更新标志
    uint8_t  alignmentflag ; //校准是否完成
    uint8_t* byte8img;   //8位图像
    uint16_t* byte12img; //12位图像
    fingertech_resutlt_t fingertechfpresult;//返回测试结果
} fingertech_fpdevice_t;




//硬件GPIO口定义
#define FINGERTECH_FPRST_GPIO						GPIO3
#define FINGERTECH_FPRST_GPIO_PIN				    (17U)
#define FINGERTECH_FPRST_IOMUXC					    IOMUXC_GPIO_SD_B0_05_GPIO3_IO17


void FingerTech_Spigpioinit(void);
void FingerTech_SpiConfigInit(void);
void FingerTech_SpiWRDate(uint8_t* wdata,uint8_t* rdata,size_t size);
void FingerTech_FpRstInit(void);
void FingerTech_FpRstHigh(void);
void FingerTech_FpRstLow(void);
uint8_t FingerTecg_ReadFpSensorID(void);
uint8_t FingerTecg_SetFpIntMode(uint8_t intagc);
uint8_t FingerTecg_SetFpWorkMode(uint8_t workagc);
void FingerTecg_SetFpSleepMode(void);
uint8_t FingerTech_GetFpImage(uint8_t* imgbuf);
void Fingertech_Imagedatamerge(uint8_t* imgsrcbuf,uint16_t* imgdesbuf);
uint16_t Fingertech_Calcimageagv(uint16_t* imgbuf);
void FingerTech_SpiReadDate(uint8_t* rdata,size_t size);

#endif
