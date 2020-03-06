#include "main.h"
#include "FingerTech_common.h"
#include "FingerTech_spiConfig.h"
#include "FingerTech_camera.h"

unsigned char flag = ZERO;
volatile bool fg_command = true;
volatile bool isEnroll = false;

edma_handle_t txHandle;
edma_handle_t rxHandle;
lpspi_slave_edma_handle_t g_s_handle;

/**************************************
* @brief eDMA Callback Function: call it when DMA transfer has completed
*	
*	@param  LPSPI base address
*	@param	The handle of lpspi_slave edma
*	@param	The status of callback function
*	
*	@return
****************************************/
void EDMA_Callback(LPSPI_Type *base,lpspi_slave_edma_handle_t *handle,status_t status,void *userData)
{
	if (status == kStatus_Success){
		__NOP();  
	}
	
	DEBUG("slaveRxData[0] = 0x%02x",slaveRxData[0]);
	if(fg_command){
		switch(slaveRxData[0]){
			case READ_IMAGE: {
				DEBUG("start read image");
				fg_command = false;
				flag = READ_IMAGE;
			} 
			break;
			default: {
				DEBUG("error command");
				DMA_transfer_data(slaveTxData,slaveRxData,32);
			}
			break;
		}
	}else{
		DEBUG("DMA transfer finished");
		GPIO_PinWrite(INT_GPIO,INT_GPIO_PIN,0U);
		fg_command = true;
		DMA_transfer_data(slaveTxData,slaveRxData,32);
		LPSPI_EnableDMA(LPSPI_SLAVE_BASEADDR, kLPSPI_RxDmaEnable | kLPSPI_TxDmaEnable);
		LPSPI_Enable(LPSPI_SLAVE_BASEADDR, true);
	}
	
	return;
}

/**************************************
* @brief eDMA transfer function
*	
*	@param	buffer address to send 
*	@param	buffer address to receive
*	@param	transfer data length
*	
*	@return
****************************************/
lpspi_transfer_t myslaveXfer;
void DMA_transfer_data(void *tx,void *rx,uint32_t length)
{
	myslaveXfer.txData = (uint8_t *)tx;
	myslaveXfer.rxData = (uint8_t *)rx;
	myslaveXfer.dataSize = length;
	myslaveXfer.configFlags = LPSPI_SLAVE_PCS_FOR_TRANSFER | kLPSPI_SlaveByteSwap;
	
	LPSPI_SlaveTransferEDMA(LPSPI_SLAVE_BASEADDR,&g_s_handle,&myslaveXfer);
}

/**************************************
* @brief config spi and DMA
*	
*	@return
****************************************/
void Fingertech_SpiConfig(void)
{
	edma_config_t config;
	lpspi_slave_config_t slaveConfig;
	
	/*Set clock source for LPSPI*/
	CLOCK_SetMux(kCLOCK_LpspiMux, LPSPI_CLOCK_SOURCE_SELECT);
	CLOCK_SetDiv(kCLOCK_LpspiDiv, LPSPI_CLOCK_SOURCE_DIVIDER);

	DEBUG("LPSPI board to board functional interrupt example.");
	DEBUG("1111  Slave start to receive data...");
	
	/*Slave config*/
	slaveConfig.bitsPerFrame = 32;
	slaveConfig.cpol = kLPSPI_ClockPolarityActiveHigh;
	slaveConfig.cpha = kLPSPI_ClockPhaseFirstEdge;
	slaveConfig.direction = kLPSPI_MsbFirst;

	slaveConfig.whichPcs = LPSPI_SLAVE_PCS_FOR_INIT;
	slaveConfig.pcsActiveHighOrLow = kLPSPI_PcsActiveLow;

	slaveConfig.pinCfg = kLPSPI_SdiInSdoOut;//kLPSPI_SdoInSdiOut;//
	slaveConfig.dataOutConfig = kLpspiDataOutRetained;

	LPSPI_SlaveInit(LPSPI_SLAVE_BASEADDR, &slaveConfig);
	
	gpio_pin_config_t int_config = {kGPIO_DigitalOutput,0,kGPIO_NoIntmode};
	GPIO_PinInit(INT_GPIO,INT_GPIO_PIN,&int_config);
	GPIO_PinWrite(INT_GPIO,INT_GPIO_PIN,0U);
	
	/*DMA config*/
	DMAMUX_Init(LP_DMAMUX);
	EDMA_GetDefaultConfig(&config);
	EDMA_Init(LP_DMA, &config);

	DMAMUX_SetSource(LP_DMAMUX, SPI_TX_DMA_LPSPI_CHANNEL, TX_DMA_SOURCE);
	DMAMUX_SetSource(LP_DMAMUX, SPI_RX_DMA_LPSPI_CHANNEL, RX_DMA_SOURCE);
	DMAMUX_EnableChannel(LP_DMAMUX, SPI_TX_DMA_LPSPI_CHANNEL);
	DMAMUX_EnableChannel(LP_DMAMUX, SPI_RX_DMA_LPSPI_CHANNEL);
	EDMA_CreateHandle(&txHandle, LP_DMA, SPI_TX_DMA_LPSPI_CHANNEL);
	EDMA_CreateHandle(&rxHandle, LP_DMA, SPI_RX_DMA_LPSPI_CHANNEL);
	
	LPSPI_SlaveTransferCreateHandleEDMA(LPSPI_SLAVE_BASEADDR,&g_s_handle,EDMA_Callback,NULL,&rxHandle,&txHandle);

	DMA_transfer_data((void *)slaveTxData,(void *)slaveRxData,32);
	LPSPI_EnableDMA(LPSPI_SLAVE_BASEADDR, kLPSPI_RxDmaEnable | kLPSPI_TxDmaEnable);
  LPSPI_Enable(LPSPI_SLAVE_BASEADDR, true);
	DEBUG("spi config done");
}

/**************************************
* @brief Change exposure time
*	
*	@return
****************************************/
void FingerTech_change_exposureTime(int i)
{
	uint8_t np = 0,nb = 0;
	uint32_t temp = i*33750/camera_cfg.full_size_w;
	
	np=temp>>8&0xff;
	nb=(uint8_t)temp&0xff;
	
	if(temp < 1000)
		temp = 1000;
	M1_V1__full_frame_size(camera_cfg.full_size_w,temp+1);
//	M1_V1__full_frame_size(800,1604);
	
	Fingertech_sp0828iicwritedata(0x3100,np);
	Fingertech_sp0828iicwritedata(0x3101,nb);
}

/**************************************
* @brief Change exposure gain
*	
*	@return
****************************************/
void FingerTech_change_exposureGain(void)
{
	DEBUG("gain = 0x%02x\n",slaveRxData[2]);
	Fingertech_sp0828iicwritedata(0x3102, slaveRxData[2]);
}


/**************************************
* @brief Adjust center point coordinates
*	
*	@return
****************************************/
int FingerTech_move_image(uint16_t *img,int height, int width)
{
	int ret=0;
	uint8_t oricoor[4]={0};
	
	ret=coordinate(img,height,width,oricoor);
	
	camera_cfg.x +=oricoor[3]|oricoor[2]<<8;
	camera_cfg.y +=oricoor[1]|oricoor[0]<<8;

	return ret;
	
}