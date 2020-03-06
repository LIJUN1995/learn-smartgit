#include "main.h"

/* Select USB1 PLL (480 MHz) as master lpi2c clock source */
#define LPI2C_CLOCK_SOURCE_SELECT (0U)
/* Clock divider for master lpi2c clock source */
#define LPI2C_CLOCK_SOURCE_DIVIDER (5U)

#define LPI2C_CLOCK_FREQUENCY ((CLOCK_GetFreq(kCLOCK_Usb1PllClk) / 8) / (LPI2C_CLOCK_SOURCE_DIVIDER + 1U))

#define RT1052_CCMCCOSR      (*(volatile unsigned long *)0x400FC060)


/***********************************
 * 初始化gpio
 **********************************/
void Fingertech_sp0828gpioinit(void)
{
    CLOCK_EnableClock(kCLOCK_Iomuxc);          
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_00_LPI2C1_SCL,1); 
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_01_LPI2C1_SDA,1); 
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B1_00_LPI2C1_SCL,0xF8B0u); 
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B1_01_LPI2C1_SDA,0xF8B0u);  
}

/***********************************
 * 初始化IIC接口
 **********************************/
void Fingertech_sp0828iicconfig(void)
{
    lpi2c_master_config_t  masterconfig;
    //配置时钟
    CLOCK_SetMux(kCLOCK_Lpi2cMux, LPI2C_CLOCK_SOURCE_SELECT);
    CLOCK_SetDiv(kCLOCK_Lpi2cDiv, LPI2C_CLOCK_SOURCE_DIVIDER);
    //进行默认配置
    LPI2C_MasterGetDefaultConfig(&masterconfig);
    //设置通讯速度
    masterconfig.baudRate_Hz = 300000U;
    LPI2C_MasterInit(LPI2C1,&masterconfig, LPI2C_CLOCK_FREQUENCY);
   // LPI2C_MasterEnable(FINGERTECH_I2C_MASTER,true);
}

/***********************************
 * IIC发送数据接口
 **********************************/
void Fingertech_sp0828iicwritedata(uint16_t regaddress,uint8_t data)
{
    uint8_t senddata[1];
    lpi2c_master_transfer_t masterxfer={0} ;
    senddata[0] = data;
    masterxfer.slaveAddress = 0x30;//0x30; 0011 0000 00011000
    masterxfer.direction = kLPI2C_Write;
    masterxfer.subaddress = regaddress;
    masterxfer.subaddressSize = 2; //地址长度一共字节
    masterxfer.data = &senddata[0];
    masterxfer.dataSize = 1;
    masterxfer.flags = kLPI2C_TransferDefaultFlag;
    LPI2C_MasterTransferBlocking(LPI2C1,&masterxfer); 
}

/***********************************
 * IIC读数据数据接口
 **********************************/
void Fingertech_sp0828iicreaddata(uint16_t regaddress,void *readval)
{
    lpi2c_master_transfer_t masterxfer ;
    masterxfer.slaveAddress = 0x30;//0x30; 0011 0001
    masterxfer.direction = kLPI2C_Read;
    masterxfer.subaddress = (uint32_t)regaddress;
    masterxfer.subaddressSize = 2;
    masterxfer.data = readval;
    masterxfer.dataSize =1;
    masterxfer.flags = kLPI2C_TransferDefaultFlag;
    LPI2C_MasterTransferBlocking(LPI2C1,&masterxfer);
}





