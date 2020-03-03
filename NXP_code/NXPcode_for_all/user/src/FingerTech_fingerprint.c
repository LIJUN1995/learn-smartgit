#include "main.h"

//读取图像发送buf
uint8_t fingertech_rdbuftx[FINGERTECH_READIMGSIZE]={0x00};
//读取图像接收buf
uint8_t fingertech_rdbufrx[FINGERTECH_READIMGSIZE]={0x00};

/****************************
功能: spi gpio 初始化
说明:
****************************/
void FingerTech_Spigpioinit(void)
{
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_SD_B0_00_LPSPI1_SCK,        /* GPIO_SD_B0_00 is configured as LPSPI1_SCK */
        0U);                                    /* Software Input On Field: Input Path is determined by functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_SD_B0_01_LPSPI1_PCS0,       /* GPIO_SD_B0_01 is configured as LPSPI1_PCS0 */
        0U);                                    /* Software Input On Field: Input Path is determined by functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_SD_B0_02_LPSPI1_SDO,        /* GPIO_SD_B0_02 is configured as LPSPI1_SDO */
        0U);                                    /* Software Input On Field: Input Path is determined by functionality */
    IOMUXC_SetPinMux(
        IOMUXC_GPIO_SD_B0_03_LPSPI1_SDI,        /* GPIO_SD_B0_03 is configured as LPSPI1_SDI */
        0U);                                    /* Software Input On Field: Input Path is determined by functionality */
    IOMUXC_SetPinConfig(
        IOMUXC_GPIO_SD_B0_00_LPSPI1_SCK,        /* GPIO_SD_B0_00 PAD functional properties : */
        0x10B0u);                               /* Slew Rate Field: Slow Slew Rate
                                                    Drive Strength Field: R0/6
                                                    Speed Field: medium(100MHz)
                                                    Open Drain Enable Field: Open Drain Disabled
                                                    Pull / Keep Enable Field: Pull/Keeper Enabled
                                                    Pull / Keep Select Field: Keeper
                                                    Pull Up / Down Config. Field: 100K Ohm Pull Down
                                                    Hyst. Enable Field: Hysteresis Disabled */
    IOMUXC_SetPinConfig(
        IOMUXC_GPIO_SD_B0_01_LPSPI1_PCS0,       /* GPIO_SD_B0_01 PAD functional properties : */
        0x10B0u);                               /* Slew Rate Field: Slow Slew Rate
                                                    Drive Strength Field: R0/6
                                                    Speed Field: medium(100MHz)
                                                    Open Drain Enable Field: Open Drain Disabled
                                                    Pull / Keep Enable Field: Pull/Keeper Enabled
                                                    Pull / Keep Select Field: Keeper
                                                    Pull Up / Down Config. Field: 100K Ohm Pull Down
                                                    Hyst. Enable Field: Hysteresis Disabled */
    IOMUXC_SetPinConfig(
        IOMUXC_GPIO_SD_B0_02_LPSPI1_SDO,        /* GPIO_SD_B0_02 PAD functional properties : */
        0x10B0u);                               /* Slew Rate Field: Slow Slew Rate
                                                    Drive Strength Field: R0/6
                                                    Speed Field: medium(100MHz)
                                                    Open Drain Enable Field: Open Drain Disabled
                                                    Pull / Keep Enable Field: Pull/Keeper Enabled
                                                    Pull / Keep Select Field: Keeper
                                                    Pull Up / Down Config. Field: 100K Ohm Pull Down
                                                    Hyst. Enable Field: Hysteresis Disabled */
    IOMUXC_SetPinConfig(
        IOMUXC_GPIO_SD_B0_03_LPSPI1_SDI,        /* GPIO_SD_B0_03 PAD functional properties : */
        0x10B0u);                               /* Slew Rate Field: Slow Slew Rate
                                                    Drive Strength Field: R0/6
                                                    Speed Field: medium(100MHz)
                                                    Open Drain Enable Field: Open Drain Disabled
                                                    Pull / Keep Enable Field: Pull/Keeper Enabled
                                                    Pull / Keep Select Field: Keeper
                                                    Pull Up / Down Config. Field: 100K Ohm Pull Down
                                                    Hyst. Enable Field: Hysteresis Disabled */
}

/****************************
功能: spi 初始化
说明: 3.5M 8位 模式0  MISO 输出 MOSI 输入
****************************/
void FingerTech_SpiConfigInit(void)
{
	lpspi_master_config_t masterConfig;
	uint32_t srcClock_Hz;
	/*Set clock source for LPSPI*/
    //选择时钟源
	//CLOCK_SetMux(kCLOCK_LpspiMux, EXAMPLE_LPSPI_CLOCK_SOURCE_SELECT);
	//CLOCK_SetMux(kCLOCK_LpspiMux, 0);
	//CLOCK_SetDiv(kCLOCK_LpspiDiv, EXAMPLE_LPSPI_CLOCK_SOURCE_DIVIDER);

    CLOCK_SetMux(kCLOCK_LpspiMux, 1);
	CLOCK_SetDiv(kCLOCK_LpspiDiv, 5);

	masterConfig.baudRate = (4000000U);//(10000000U);//(3500000U);
	masterConfig.bitsPerFrame = 8;
	masterConfig.cpol = kLPSPI_ClockPolarityActiveHigh;//;//kLPSPI_ClockPolarityActiveLow
	masterConfig.cpha =  kLPSPI_ClockPhaseSecondEdge;//kLPSPI_ClockPhaseFirstEdge;
	masterConfig.direction = kLPSPI_MsbFirst;

//	masterConfig.pcsToSckDelayInNanoSec = 10;
//	masterConfig.lastSckToPcsDelayInNanoSec = 10;
//	masterConfig.betweenTransferDelayInNanoSec = 10;
	masterConfig.pcsToSckDelayInNanoSec = 1000000000 / masterConfig.baudRate;
	masterConfig.lastSckToPcsDelayInNanoSec = 1000000000 / masterConfig.baudRate;
	masterConfig.betweenTransferDelayInNanoSec = 1000000000 / masterConfig.baudRate;

	masterConfig.whichPcs = kLPSPI_Pcs0;			//使用片选0
	masterConfig.pcsActiveHighOrLow = kLPSPI_PcsActiveLow;//0表示选中

	masterConfig.pinCfg = kLPSPI_SdiInSdoOut;//kLPSPI_SdoInSdiOut; //kLPSPI_SdiInSdoOut
	masterConfig.dataOutConfig = kLpspiDataOutTristate;//取消选中后保持高阻态

	//srcClock_Hz = LPSPI_MASTER_CLK_FREQ;
    srcClock_Hz = (CLOCK_GetFreq(kCLOCK_Usb1PllPfd1Clk)/(3+1));
	LPSPI_MasterInit(LPSPI1, &masterConfig, srcClock_Hz);
	LPSPI_Enable(LPSPI1,true);		
}

/****************************
功能: spi 读写数据
说明:
****************************/
void FingerTech_SpiWRDate(uint8_t* wdata,uint8_t* rdata,size_t size)
{
	lpspi_transfer_t fingertechmasterXfer;
	fingertechmasterXfer.txData = wdata;
	fingertechmasterXfer.rxData = rdata;
	fingertechmasterXfer.dataSize = size;
	fingertechmasterXfer.configFlags = kLPSPI_MasterPcs0 | kLPSPI_MasterPcsContinuous | kLPSPI_SlaveByteSwap;
	LPSPI_MasterTransferBlocking(LPSPI1, &fingertechmasterXfer);
}

/****************************
功能: spi 读数据
说明:
****************************/
void FingerTech_SpiReadDate(uint8_t* rdata,size_t size)
{
	lpspi_transfer_t fingertechmasterXfer;
	fingertechmasterXfer.rxData = rdata;
	fingertechmasterXfer.dataSize = size;
	fingertechmasterXfer.configFlags = kLPSPI_MasterPcs0 | kLPSPI_MasterPcsContinuous | kLPSPI_SlaveByteSwap;
	LPSPI_MasterTransferBlocking(LPSPI1, &fingertechmasterXfer);
}

/****************************
功能: RST 初始化
说明:
****************************/
void FingerTech_FpRstInit(void)
{
    gpio_pin_config_t  fingertech_fpresetpin = {kGPIO_DigitalOutput,0,kGPIO_NoIntmode};
    IOMUXC_SetPinMux(FINGERTECH_FPRST_IOMUXC,0U);                                    
    IOMUXC_SetPinConfig(FINGERTECH_FPRST_IOMUXC,0x10B0u);
    GPIO_PinInit(FINGERTECH_FPRST_GPIO,FINGERTECH_FPRST_GPIO_PIN,&fingertech_fpresetpin);
}

/****************************
功能: RST 输出高
说明:
****************************/
void FingerTech_FpRstHigh(void)
{
    GPIO_PinWrite(FINGERTECH_FPRST_GPIO,FINGERTECH_FPRST_GPIO_PIN,1);
}

/****************************
功能: RST 输出低
说明:
****************************/
void FingerTech_FpRstLow(void)
{
    GPIO_PinWrite(FINGERTECH_FPRST_GPIO,FINGERTECH_FPRST_GPIO_PIN,0);
}

/****************************
功能: 获取ID
说明:
****************************/
uint8_t FingerTecg_ReadFpSensorID(void)
{
  static uint8_t resetcmd = FINGERTECH_RESETCMD;
  static uint8_t startcmd = FINGERTECH_STARTCMD;
  uint8_t id_tx[4] = {FINGERTECH_READCHIPID,FINGERTECH_DUMMY,FINGERTECH_DUMMY,FINGERTECH_DUMMY};
  uint8_t id_rx[4] = {0x00};
  FingerTech_FpRstLow();
  FingerTech_Delayms(10);
  FingerTech_FpRstHigh();
  FingerTech_Delayms(10);
  FingerTech_SpiWRDate(&resetcmd,id_rx,1);
  FingerTech_Delayms(1);
  FingerTech_SpiWRDate(&resetcmd,id_rx,1);
  FingerTech_Delayms(1);
  FingerTech_SpiWRDate(&resetcmd,id_rx,1);
  FingerTech_Delayms(1);
  FingerTech_SpiWRDate(&startcmd,id_rx,1);
  FingerTech_Delayms(1);
  FingerTech_SpiWRDate(&startcmd,id_rx,1);
  FingerTech_Delayms(1);
  FingerTech_SpiWRDate(&id_tx[0],&id_rx[0],4);
  if(0x98 != id_rx[3])
  {
	  FingerTech_Logd("sensor chipid err %x",id_rx[3]);
      return FINGERTECH_READIDERR;
  }
  FingerTech_Logd("sensor chipid is fps998ea");
  return FINGERTECH_SUCCESS;
}


/****************************
功能: 工作模式配置
说明:
****************************/
uint8_t FingerTecg_SetFpWorkMode(uint8_t workagc)
{
  uint8_t rdbuf[7]={0x00};
  static uint8_t resetcmd = FINGERTECH_RESETCMD;
  static uint8_t startcmd = FINGERTECH_STARTCMD;
  static uint8_t reg21[7]= {0x21,0x66,0x66,0x36,0x00,0xff,0xf0};
  static uint8_t reg27[7]= {0x27,0x66,0x66,0x24,0x5a,0x7b,0x13};

  reg21[6] = workagc;
  FingerTech_SpiWRDate(&resetcmd,rdbuf,1);
  FingerTech_Delayus(100);
  FingerTech_SpiWRDate(&resetcmd,rdbuf,1);
  FingerTech_Delayus(100);
  FingerTech_SpiWRDate(&resetcmd,rdbuf,1);
  FingerTech_Delayus(100);
  FingerTech_SpiWRDate(&startcmd,rdbuf,1);
  FingerTech_Delayus(100);
  FingerTech_SpiWRDate(reg21,rdbuf,7);
  FingerTech_SpiWRDate(reg21,rdbuf,7);
  if(rdbuf[6] != reg21[6])
  {
      return FINGERTECH_CONFIGREGERR;
  }
  FingerTech_SpiWRDate(reg27,rdbuf,7);
  FingerTech_SpiWRDate(reg27,rdbuf,7);
  return FINGERTECH_SUCCESS;
}


/****************************
功能: 中断模式配置
说明:
****************************/
uint8_t FingerTecg_SetFpIntMode(uint8_t intagc)
{
    uint8_t rdbuf[9]={0x00};
    static uint8_t resetcmd = FINGERTECH_RESETCMD;
    static uint8_t startcmd = FINGERTECH_STARTCMD;
    static uint8_t intcmd   = 0x14;
    static uint8_t reg21[7]=  {0x21,0x66,0x66,0x36,0x00,0xff,0xf0};
    static uint8_t reg22[9]=  {0x22,0x66,0x66,0x20,0x08,0x20,0x08,0x30,0x07};
    static uint8_t reg27[7]=  {0x27,0x66,0x66,0x24,0x6a,0x7b,0x13};

    reg21[6] = intagc;
    FingerTech_SpiWRDate(&resetcmd,rdbuf,1);
    FingerTech_Delayus(100);
    FingerTech_SpiWRDate(&resetcmd,rdbuf,1);
    FingerTech_Delayus(100);
    FingerTech_SpiWRDate(&resetcmd,rdbuf,1);
    FingerTech_Delayus(100);
    FingerTech_SpiWRDate(&startcmd,rdbuf,1);
    FingerTech_Delayus(100);
    FingerTech_SpiWRDate(reg21,rdbuf,7);
    FingerTech_SpiWRDate(reg21,rdbuf,7);
    if(rdbuf[6] != reg21[6])
    {
         return FINGERTECH_CONFIGREGERR;
    }
    FingerTech_SpiWRDate(reg22,rdbuf,9);
    FingerTech_SpiWRDate(reg22,rdbuf,9); 
    if(rdbuf[8] != reg22[8])
    {
         return FINGERTECH_CONFIGREGERR;
    }   
    FingerTech_SpiWRDate(reg27,rdbuf,7);
    FingerTech_SpiWRDate(reg27,rdbuf,7);
    if(reg27[6] != reg27[6])
    {
         return FINGERTECH_CONFIGREGERR;
    }  
    //发进入中断模式指令
    FingerTech_SpiWRDate(&intcmd,rdbuf,1);
    return FINGERTECH_SUCCESS;
}

/****************************
功能: 睡眠模式配置
说明:
****************************/
void FingerTecg_SetFpSleepMode(void)
{
    static uint8_t sleepcmd = FINGERTECH_SLEEPCMD;
    uint8_t retval = 0;
    FingerTech_SpiWRDate(&sleepcmd,&retval,1);
}

/****************************
功能: 获取图像
说明:
****************************/
uint8_t FingerTech_GetFpImage(uint8_t* imgbuf)
{
    uint32_t count=0;
    int temp =0;
    static uint8_t imgheadlen = 4;
    uint8_t resetlinecmd = 0xa0;
    uint8_t retval=0;
    uint8_t imghead[4] = {0xaa,0xaa,0x0a,0xaa};
    FingerTech_SpiWRDate(&resetlinecmd,&retval,1);
    memset(&fingertech_rdbuftx[0],0x66,FINGERTECH_READIMGSIZE);
    memset(&fingertech_rdbufrx[0],0x00,FINGERTECH_READIMGSIZE);
    fingertech_rdbuftx[0] = 0x90;
    FingerTech_SpiWRDate(&fingertech_rdbuftx[0],&fingertech_rdbufrx[0],FINGERTECH_READIMGSIZE);
	for(count=0;count<(FINGERTECH_READIMGSIZE - IMG_WIDTH - imgheadlen);count++)
	{
		if((fingertech_rdbufrx[count] == imghead[imgheadlen -2]) && \
		(fingertech_rdbufrx[count +(imgheadlen/2 - 1)] == imghead[(imgheadlen -1)]))
		{
			count += imgheadlen;
			temp = fingertech_rdbufrx[count - 1];
			memcpy((imgbuf + temp*IMG_WIDTH),(fingertech_rdbufrx + count),IMG_WIDTH);
			count += IMG_WIDTH;
			if(temp == IMG_HEIGHT -1)
			{
				return FINGERTECH_SUCCESS;
			}
		}
	}
    memset(&fingertech_rdbuftx[0],0x66,FINGERTECH_READIMGSIZE);
    memset(&fingertech_rdbufrx[0],0x00,FINGERTECH_READIMGSIZE);
    fingertech_rdbuftx[0] = 0x90;
    FingerTech_SpiWRDate(&fingertech_rdbuftx[0],&fingertech_rdbufrx[0],FINGERTECH_READIMGSIZE);
    return FINGERTECH_READIMGERR;
}

/****************************
功能: 将图像数据合成12bit
说明: imgsrcbuf 原数据,合并后的12bit数据
****************************/
void Fingertech_Imagedatamerge(uint8_t* imgsrcbuf,uint16_t* imgdesbuf)
{
    uint32_t count = 0;
    for(count=0;count<FINGERTECH_IMAGESIZE/2;count++)
    {
        imgdesbuf[count]=imgsrcbuf[2*count]*256 + imgsrcbuf[2*count +1];
    }
}

/****************************
功能: 计算图像平均值
说明: imgbuf 图像数据
****************************/
uint16_t Fingertech_Calcimageagv(uint16_t* imgbuf)
{
    uint32_t count = 0;
    uint64_t sum = 0;
    for(count=0;count<FINGERTECH_IMAGESIZE/2;count++)
    {
        sum += imgbuf[count] ;
    }
    return sum/(FINGERTECH_IMAGESIZE/2);
}








