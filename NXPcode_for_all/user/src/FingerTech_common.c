#include "main.h"
#include "FingerTech_Exit.h"
#include "FingerTech_spiConfig.h"
#include "FingerTech_camera.h"
#include "FingerTech_spiConfig.h"

AT_NONCACHEABLE_SECTION_INIT(uint8_t slaveRxData[32]) = {0U};
AT_NONCACHEABLE_SECTION_INIT(uint8_t slaveTxData[32]) = {0U};

#ifdef P1801
AT_NONCACHEABLE_SECTION_INIT(uint16_t DeNoise_Rx[TRANSFER_SIZE]) = {0U};
AT_NONCACHEABLE_SECTION_INIT(uint16_t DeNoise_Tx[TRANSFER_SIZE]) = {0U};
AT_CACHEABLE_SECTION_ALIGN(uint16_t s_frameBuffer[TRANSFER_SIZE],64);
#endif 

#ifdef P1701
AT_NONCACHEABLE_SECTION_INIT(uint8_t DeNoise_Tx[W*H]) = {0U};
AT_NONCACHEABLE_SECTION_INIT(uint8_t DeNoise_Rx[W*H]) = {0U};
AT_CACHEABLE_SECTION_ALIGN(uint8_t s_frameBuffer[TRANSFER_SIZE_O],64);
#endif 

extern volatile bool fg_command;
volatile bool retry_flag = true;
	/****************************
功能: 1ms 延时函数
说明:
****************************/
void FingerTech_Delayms(uint16_t nms)
{
	uint32_t i;
	volatile uint32_t j;
	for(i=0;i<nms;i++) 
	{
	 for(j=0;j<120000;j++)
	 {
		__asm("NOP");
	 }
	}
}
void Cdfinger_ChangeExposureTime(void)
{
	camera_cfg.exp_time=slaveRxData[2];
	update_camera_cfg();
	camera_ptr.init();
	flag = ZERO;
	fg_command = true;
	memset(slaveRxData,0,sizeof(slaveRxData));
	DMA_transfer_data(slaveTxData,slaveRxData,sizeof(slaveTxData));
	LPSPI_EnableDMA(LPSPI_SLAVE_BASEADDR, kLPSPI_RxDmaEnable | kLPSPI_TxDmaEnable);
  LPSPI_Enable(LPSPI_SLAVE_BASEADDR, true);
}
	
void Cdfinger_MoveImage(void)
{
	uint32_t status = 0;
	uint32_t i = 0;
	
	do{
		FingerTech_GetImg();
		for(i=0;i<camera_cfg.hight*camera_cfg.width;i++)
			s_frameBuffer[i] &= 0x0fff;
		status = FingerTech_move_image((uint16_t *)s_frameBuffer,camera_cfg.hight, camera_cfg.width);
		camera_ptr.init();
		CSI_ReflashFifoDma(CSI, kCSI_RxFifo);
		CSI_Start(CSI);
	}while(status);
	update_camera_cfg();
	
	flag = ZERO;
	fg_command = true;
	memset(slaveRxData,0,sizeof(slaveRxData));
	DMA_transfer_data(slaveTxData,slaveRxData,sizeof(slaveTxData));
	LPSPI_EnableDMA(LPSPI_SLAVE_BASEADDR, kLPSPI_RxDmaEnable | kLPSPI_TxDmaEnable);
  LPSPI_Enable(LPSPI_SLAVE_BASEADDR, true);
}

void Cdfinger_GetImage(uint8_t num)
{
	uint32_t i = 0;
	
	if((num==0) || (num>=6))
		num = 1;
	
	memset(DeNoise_Tx,0x00,sizeof(DeNoise_Tx));
	for(i=0;i<num;i++){
		uint8_t status = 0;
		
		FingerTech_GetImg();
	
		L1CACHE_InvalidateDCacheByRange((uint32_t)s_frameBuffer,sizeof(s_frameBuffer));
		
		if(camera_cfg.stat == PIS1801){
			FusionImg(DeNoise_Tx,s_frameBuffer,camera_cfg.hight*camera_cfg.width, i+1, num-1-i);
			do{
				Fingertech_sp0828iicreaddata(0x3013,&status);
			}while(status != 0x02);
		}else if(camera_cfg.stat == PIS1701){
			one_Extrect_three(s_frameBuffer, DeNoise_Tx, M_H, M_W,i+1);
		}
	}
	L1CACHE_CleanDCacheByRange((uint32_t)DeNoise_Tx,sizeof(DeNoise_Tx));
}

void Cdfinger_EnrollPrepareTransfer(void)
{
	Cdfinger_GetImage(slaveRxData[3]);
	DMA_transfer_data((void *)DeNoise_Tx,(void *)DeNoise_Rx,sizeof(DeNoise_Tx));
	flag = ZERO;
	memset(slaveRxData,0,sizeof(slaveRxData));
	GPIO_PinWrite(INT_GPIO,INT_GPIO_PIN,1U);
	
	LPSPI_EnableDMA(LPSPI_SLAVE_BASEADDR, kLPSPI_RxDmaEnable | kLPSPI_TxDmaEnable);
  LPSPI_Enable(LPSPI_SLAVE_BASEADDR, true);
}

void Cdfinger_RetryRecPrepareTransfer(void)
{
	while(retry_flag == true);
	DMA_transfer_data((void *)DeNoise_Tx,(void *)DeNoise_Rx,sizeof(DeNoise_Tx));
	GPIO_PinWrite(INT_GPIO,INT_GPIO_PIN,1U);
	retry_flag = true;
}

void Cdfinger_RecPrepareTransfer(void)
{
	Cdfinger_GetImage(slaveRxData[3]);
	DMA_transfer_data((void *)DeNoise_Tx,(void *)DeNoise_Rx,sizeof(DeNoise_Tx));
	GPIO_PinWrite(INT_GPIO,INT_GPIO_PIN,1U);
	
	while(!fg_command);
	Cdfinger_GetImage(slaveRxData[3]);
	DMA_transfer_data(slaveTxData,slaveRxData,32);
	retry_flag = false;
}

void Cdfinger_AttributeTransfer(void)
{
	int a = sizeof(chip_cfg);
	flag = ZERO;
	L1CACHE_CleanDCacheByRange((uint32_t)&camera_cfg,sizeof(chip_cfg));
	DMA_transfer_data((void *)&camera_cfg,(void *)DeNoise_Rx,sizeof(chip_cfg));
	memset(slaveRxData,0,sizeof(slaveRxData));
	GPIO_PinWrite(INT_GPIO,INT_GPIO_PIN,1U);
	
	LPSPI_EnableDMA(LPSPI_SLAVE_BASEADDR, kLPSPI_RxDmaEnable | kLPSPI_TxDmaEnable);
  LPSPI_Enable(LPSPI_SLAVE_BASEADDR, true);
}

void Cdfinger_SetCoordinates(void)
{
	camera_cfg.y=((slaveRxData[2]<<8)&0xff00) | (slaveRxData[3]&0x00ff);
	camera_cfg.x1=((slaveRxData[4]<<8)&0xff00) | (slaveRxData[5]&0x00ff);
	camera_cfg.x2=((slaveRxData[6]<<8)&0xff00) | (slaveRxData[7]&0x00ff);
	camera_cfg.x3=((slaveRxData[8]<<8)&0xff00) | (slaveRxData[9]&0x00ff);
	update_camera_cfg();
	camera_ptr.init();
	flag = ZERO;
	fg_command = true;
	memset(slaveRxData,0,sizeof(slaveRxData));
	DMA_transfer_data(slaveTxData,slaveRxData,sizeof(slaveTxData));
	LPSPI_EnableDMA(LPSPI_SLAVE_BASEADDR, kLPSPI_RxDmaEnable | kLPSPI_TxDmaEnable);
  LPSPI_Enable(LPSPI_SLAVE_BASEADDR, true);
}
 
/****************************
功能: 1us 延时函数
说明:
****************************/
void FingerTech_Delayus(uint32_t nus)
{		
	uint32_t i;
	volatile uint32_t j;
	for(i=0;i<nus;i++) 
	{
	 for(j=0;j<120;j++)
	 {
		__asm("NOP");
	 }
	}
}
  



/****************************
功能: LED 初始化函数
说明:
****************************/
void FingerTech_LedInit(void)
{
	gpio_pin_config_t  fingertech_led = {kGPIO_DigitalOutput,0,kGPIO_NoIntmode};
	 
	IOMUXC_SetPinMux(FINGERTECH_LED_IOMUXC,0U); 
	IOMUXC_SetPinConfig(FINGERTECH_LED_IOMUXC,0x10B0u);  
	
	GPIO_PinInit(FINGERTECH_LED_GPIO,FINGERTECH_LED_GPIO_PIN,&fingertech_led);
}

/****************************
功能: LED 灯亮
说明:
****************************/
void FingerTech_LedOn(void)
{
	GPIO_PinWrite(FINGERTECH_LED_GPIO,FINGERTECH_LED_GPIO_PIN,0);
}

/****************************
功能: LED 灯灭
说明:
****************************/
void FingerTech_LedOff(void)
{
	GPIO_PinWrite(FINGERTECH_LED_GPIO,FINGERTECH_LED_GPIO_PIN,1);
}

/****************************
功能: lpuart初始化函数
说明:
****************************/
void FingerTech_BspUartInit(void)
{
	lpuart_config_t config;
	LPUART_GetDefaultConfig(&config);
	config.baudRate_Bps = (115200U);
    config.enableTx = true;
	LPUART_Init(LPUART1, &config, BOARD_DebugConsoleSrcFreq());
}


/****************************
功能: 中断脚初始化
说明: 
****************************/
void FingerTech_FpIntPinInit(void)
{
    gpio_pin_config_t fingerintpin_config;
    IOMUXC_SetPinMux(IOMUXC_SNVS_WAKEUP_GPIO5_IO00, 0U);                                   
    IOMUXC_SetPinConfig(IOMUXC_SNVS_WAKEUP_GPIO5_IO00,0xF080);                              
    fingerintpin_config.direction = kGPIO_DigitalInput;
    fingerintpin_config.outputLogic = 0;
    fingerintpin_config.interruptMode = kGPIO_IntRisingEdge;  
    /* Init input switch GPIO. */
    GPIO_PinInit(GPIO5, 0U, &fingerintpin_config);
    FingerTech_NVIC_SetPriority(GPIO5_Combined_0_15_IRQn,4,0);
}

/****************************
功能: 指纹中断开启
说明: 
****************************/
void FingerTech_FpIntEnable(void)
{
    EnableIRQ(GPIO5_Combined_0_15_IRQn);
    GPIO_PortEnableInterrupts(GPIO5, 1U << 0U);  
}

/****************************
功能: 指纹中断关闭
说明: 
****************************/
void FingerTech_FpIntDisable(void)
{
   	DisableIRQ(GPIO5_Combined_0_15_IRQn);
    /* Enable GPIO pin interrupt */  
    //GPIO_PortEnableInterrupts(GPIO5, 1U << 0U); 
	GPIO_DisableInterrupts(GPIO5, 1U << 0U);  
}

/****************************
功能: 硬件初始化
说明:
****************************/
void FingerTech_BspInit(void)
{
	/* Init board hardware. */
	BOARD_ConfigMPU();
	BOARD_InitPins();
	BOARD_BootClockRUN();
	//设置优先级组，4个抢占优先级，0个字优先级
	FingerTech_NVIC_SetGrouping(NVIC_PRIORITYGROUP_4);
//	FingerTech_BspUartInit();
//	BOARD_InitDebugConsole();
	FingerTech_LedInit();
	Fingertech_sp0828gpioinit();
	Fingertech_sp0828iicconfig();
	FingerTech_Delayms(500);
	Fingertech_cameragpioinit();
	Fingertech_camerainitresource();
	Fingertech_cameraconfig();
}
