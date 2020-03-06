#include "main.h"
#include "FingerTech_Exit.h"
#include "FingerTech_common.h"

#define M1_DVP_INIT_num         106
extern const camera_receiver_operations_t csi_ops;
uint32_t activeFrameAddr;
uint32_t inactiveFrameAddr;
static csi_private_data_t csiPrivateData;

chip_ptr camera_ptr;
chip_cfg camera_cfg;
extern void PIS1701_Init(void);
extern void PIS1701_Start(void);
extern void PIS1701_Stop(void);

static uint16_t M1_DVP_INIT_Conf[M1_DVP_INIT_num][2] =
{
	0x3007,0x06,  // Timing config enable, Standby mode
	0x300B,0x01,  // 11bit ADC
	0X3f03,0X00,
	0x3f04,0x00,
	0x3f05,0x03,
	0x3f06,0xc0,
	0x3f07,0x00,
	0x3f08,0xe4,
	0x3f09,0x00,
	0x3f0a,0x9c,
	0x3f0b,0x03,
	0x3f0c,0xc0,
	0x3f0d,0x0f,
	0x3f0e,0xff,
	0x3f10,0x00,
	0x3f11,0xe4,
	0x3f12,0x02,
	0x3f13,0xdc,
	0x3f14,0x00,
	0x3f15,0x0c,
	0x3f16,0x00,
	0x3f17,0x90,
	0x3f18,0x00,
	0x3f19,0x48,
	0x3f1a,0x00,
	0x3f1b,0x18,
	0x3f1c,0x00,
	0x3f1d,0x0c,
	0x3f1e,0x00,
	0x3f1f,0x90,
	0x3f20,0x03,
	0x3f21,0x00,
	0x3f22,0x00,
	0x3f23,0x90,
	0x3f24,0x00,
	0x3f25,0x18,
	0x3f26,0x00,
	0x3f27,0x78,
	0x3f28,0x03,
	0x3f29,0x0c,
	0x3f2a,0x00,
	0x3f2b,0x78,
	0x3f2c,0x00,
	0x3f2d,0x18,
	0x3f2e,0x02,
	0x3f2f,0xf4,
	0x3f31,0x00,
	0x3f32,0x00,
	0x3f33,0x00,
	0x3f34,0x0c,
	0x3f35,0x00,
	0x3f36,0x00,
	0x3f37,0x03,
	0x3f38,0x0c,
	0x3f39,0x00,
	0x3f3a,0x00,
	0x3f3b,0x00,
	0x3f3c,0xb4,
	0x3f3d,0x00,
	0x3f3e,0x00,
	0x3f3f,0x00,
	0x3f40,0xc0,
	0x3f41,0x01,
	0x3f42,0x8c,
	0x3f43,0x01,
	0x3f44,0x5c,
	0x3f45,0x03,
	0x3f46,0xa8,
	0x3f47,0x09,
	0x3f48,0x60,
	0x3f49,0x01,
	0x3f4a,0x2c,
	0x3f4b,0x0b,
	0x3f4c,0xe8,
	0x3f4d,0x02,
	0x3f4e,0xdc,
	0x3f4f,0x00,
	0x3f50,0xb4,
	0x3f51,0x00,
	0x3f52,0x00,
	0x3f53,0x00,
	0x3f54,0x3c,
	0x3f55,0x00,
	0x3f56,0x00,
	0x3f57,0x00,
	0x3f58,0x60,
	0x3f59,0x00,
	0x3f5a,0x00,
	0x3f5b,0x00,
	0x3f5c,0x78,
	0x3f5e,0x00,
	0x3f5f,0x00,
	0x3f60,0x00,
	0x3f61,0x78,
	0x3f63,0x04,
	0x3f64,0x08,
	0x3f65,0x0d,
	0x3f66,0x48,  // ADC 11bit Timing, follow it!
	0x3009,0x7f,  // 
	0x3405,0x00,
	0x3900,0x00,  // DPC Disable
	0x3A00,0x00,  // Denoise Disable
	0x4006,0x0F,
	0x4027,0x10,
	0x400E,0x04,
	0x400F,0x10
};

void M1_V1_DelayMs(uint32_t ms)
{
    volatile uint32_t i;
    uint32_t loopPerMs = SystemCoreClock / 3000;

    while (ms--)
    {
        i = loopPerMs;
        while (i--)
        {
        }
    }
}

void BOARD_PullCameraResetPin(bool pullUp)
{
    /* Reset pin is connected to DCDC_3V3. */
    return;
}

void BOARD_PullCameraPowerDownPin(bool pullUp)
{
    if (pullUp)
    {
        GPIO_PinWrite(GPIO1, 18, 1);
    }
    else
    {
        GPIO_PinWrite(GPIO1, 18, 0);
    }
}

status_t FingerTech_GetImg(void)
{
	/*get one frame*/
	camera_ptr.start();

	/*wait finished*/
	while(CSI_intflag);
	
	camera_ptr.stop();
	CSI_intflag = true;
	
	return 0;
}

void Fingertech_cameragpioinit(void)
{
	CLOCK_EnableClock(kCLOCK_Iomuxc);           

	IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_02_GPIO1_IO18, 0U);                                    
                               
	IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_04_CSI_PIXCLK, 0U);                                    
	IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_05_CSI_MCLK,   0U);                                    
	IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_06_CSI_VSYNC,  0U);                                    
	IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_07_CSI_HSYNC,  0U);                                    
	IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_08_CSI_DATA09, 0U);                                    
	IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_09_CSI_DATA08, 0U);                                    
	IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_10_CSI_DATA07, 0U);                                    
	IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_11_CSI_DATA06, 0U);                                    
	IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_12_CSI_DATA05, 0U);                                    
	IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_13_CSI_DATA04, 0U);                                    
	IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_14_CSI_DATA03, 0U);                                    
	IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_15_CSI_DATA02, 0U); 
	IOMUXC_SetPinMux(IOMUXC_GPIO_B1_08_CSI_DATA11,0U);    
  IOMUXC_SetPinMux(IOMUXC_GPIO_B1_09_CSI_DATA10,0U);  
  IOMUXC_SetPinMux(IOMUXC_GPIO_B1_10_CSI_DATA00,0U);
	IOMUXC_SetPinMux(IOMUXC_GPIO_B1_11_CSI_DATA01,0U);  
  IOMUXC_SetPinMux(IOMUXC_GPIO_B1_11_CSI_DATA01,0U);        

  IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B1_02_GPIO1_IO18, 0x10B0u);
}


void Fingertech_camerainitresource(void)
{
	/* Set the pins for CSI reset and power down. */
    gpio_pin_config_t pinConfig = {kGPIO_DigitalOutput, 1,kGPIO_NoIntmode};

	/* CSI MCLK select 24M. */
    /*
     * CSI clock source:
     *
     * 00 derive clock from osc_clk (24M)
     * 01 derive clock from PLL2 PFD2
     * 10 derive clock from pll3_120M
     * 11 derive clock from PLL3 PFD1
     */
    CLOCK_SetMux(kCLOCK_CsiMux, 0);
    /*
     * CSI clock divider:
     *
     * 000 divide by 1
     * 001 divide by 2
     * 010 divide by 3
     * 011 divide by 4
     * 100 divide by 5
     * 101 divide by 6
     * 110 divide by 7
     * 111 divide by 8
     */
    CLOCK_SetDiv(kCLOCK_CsiDiv, 0);

    /*
     * For RT1050, there is not dedicate clock gate for CSI MCLK, it use CSI
     * clock gate.
     */

    GPIO_PinInit(GPIO1, 18, &pinConfig);
}


static void Fingertech_CSI_Init(const camera_config_t *config)
{
    csi_config_t csiConfig;

    CSI_GetDefaultConfig(&csiConfig);

    csiConfig.bytesPerPixel = config->bytesPerPixel;

    if (kCAMERA_InterfaceGatedClock == config->interface)
    {
        csiConfig.workMode = kCSI_GatedClockMode;
    }
    else if (kCAMERA_InterfaceNonGatedClock == config->interface)
    {
        csiConfig.workMode = kCSI_NonGatedClockMode;
    }
    else if (kCAMERA_InterfaceCCIR656 == config->interface)
    {
        csiConfig.workMode = kCSI_CCIR656ProgressiveMode;
    }
    else
    {
        return;
    }

    csiConfig.linePitch_Bytes = config->frameBufferLinePitch_Bytes;
    csiConfig.dataBus = kCSI_DataBus8Bit;
    csiConfig.useExtVsync = true;
    csiConfig.height = FSL_VIDEO_EXTRACT_HEIGHT(config->resolution);
    csiConfig.width = FSL_VIDEO_EXTRACT_WIDTH(config->resolution);

    csiConfig.polarityFlags = 0U;
    if (kCAMERA_HrefActiveHigh == (config->controlFlags & kCAMERA_HrefActiveHigh))
    {
        csiConfig.polarityFlags |= kCSI_HsyncActiveHigh;
    }
    if (kCAMERA_DataLatchOnRisingEdge == (config->controlFlags & kCAMERA_DataLatchOnRisingEdge))
    {
        csiConfig.polarityFlags |= kCSI_DataLatchOnRisingEdge;
    }
    if (kCAMERA_VsyncActiveHigh != (config->controlFlags & kCAMERA_VsyncActiveHigh))
    {
        csiConfig.polarityFlags |= kCSI_VsyncActiveLow;
    }

    CSI_Init(CSI, &csiConfig);
}

uint16_t M1_V1_readid(void)
{
	uint8_t chipid1,chipid2,chipid3,chipid4;
	
	Fingertech_sp0828iicreaddata(0x3000,&chipid1);
	Fingertech_sp0828iicreaddata(0x3001,&chipid2);
	Fingertech_sp0828iicreaddata(0x3002,&chipid3);
//	Fingertech_sp0828iicreaddata(0x3003,&chipid4);
	
	if(chipid1 == 0x08 && chipid2 == 0x01 && chipid3 == 0x00)
		return PIS1801;
	else if(chipid1 == 0x20 && chipid2 == 0x03 && chipid3 == 0x01)
		return PIS1701;
	
	return -1;
}

void M1_V1_reset(void)
{
	uint8_t rst = 0;
	
	Fingertech_sp0828iicwritedata(0x3007,0x01);
	
	Fingertech_sp0828iicreaddata(0x3007,&rst);
	//printf("reset reg=0x%02x\n",rst);
}

void M1_V1_config_mclk_pclk(void)
{
	Fingertech_sp0828iicwritedata(0x3300,0x2e);
	Fingertech_sp0828iicwritedata(0x3301,0x00);
	Fingertech_sp0828iicwritedata(0x3302,0x04);
	Fingertech_sp0828iicwritedata(0x3303,0x04);
}

void M1_V1__full_frame_size(int row, int column){
    uint8_t data;

    /*config column size*/
    data = (uint8_t)((column & 0xff00)>>8);
    Fingertech_sp0828iicwritedata(0x3200, data);
  
    data = (uint8_t)(column & 0x00ff);
    Fingertech_sp0828iicwritedata(0x3201, data);


    /*config row size*/
    data = (uint8_t)((row & 0xff00)>>8);
    Fingertech_sp0828iicwritedata(0x3202, data);
   
    data = (uint8_t)(row & 0x00ff);
    Fingertech_sp0828iicwritedata(0x3203, data);
}

void M1_V1__out_frame_size(int row_s, int row_e, int column_s, int column_e){
	uint8_t data;

	/*write to column start register*/
	data = (uint8_t)((column_s & 0xff00)>>8);
	Fingertech_sp0828iicwritedata(0x3204, data);

	data = (uint8_t)(column_s & 0x00ff);
	Fingertech_sp0828iicwritedata(0x3205, data);

	/*write to column end register*/
	data = (uint8_t)((column_e & 0xff00)>>8);
	Fingertech_sp0828iicwritedata(0x3206, data);

	data = (uint8_t)(column_e & 0x00ff);
	Fingertech_sp0828iicwritedata(0x3207, data);

	/*write to row start register*/
	data = (uint8_t)((row_s & 0xff00)>>8);
	Fingertech_sp0828iicwritedata(0x3208, data);

	data = (uint8_t)(row_s & 0x00ff);
	Fingertech_sp0828iicwritedata(0x3209, data);

	/*write to row end register*/
	data = (uint8_t)((row_e & 0xff00)>>8);
	Fingertech_sp0828iicwritedata(0x320a, data);

	data = (uint8_t)(row_e & 0x00ff);
	Fingertech_sp0828iicwritedata(0x320b, data);
}

void M1_V1_dvp_init(void){
    uint8_t i;
    uint16_t address;

    for(i = 0; i < M1_DVP_INIT_num; i++){
        //address = ((uint16_t)M1_DVP_INIT_Conf[i][0]<<8 & 0xff00) | (uint16_t)M1_DVP_INIT_Conf[i][1];
        Fingertech_sp0828iicwritedata(M1_DVP_INIT_Conf[i][0], M1_DVP_INIT_Conf[i][1]);
    }
}

void M1_V1_Init(void)
{
	uint8_t chipid1,chipid2,chipid3,chipid4;

	M1_V1_reset();	
	M1_V1_dvp_init();
	
	M1_V1_config_mclk_pclk();
	//M1_V1__full_frame_size(FULL_SIZE_W,FULL_SIZE_H);
	//M1_V1__out_frame_size(X,X+WIDTH-1,Y,Y+HEIGHT-1);//1 ºÅ»úÅäÖÃ
	FingerTech_change_exposureTime(camera_cfg.exp_time);

  M1_V1__out_frame_size(camera_cfg.x,camera_cfg.x+camera_cfg.width-1,camera_cfg.y,camera_cfg.y+camera_cfg.hight-1);//1 ºÅ»úÅäÖÃ
	/*exposure time*/
	//Fingertech_sp0828iicwritedata(0x3100, 0x07);
	//Fingertech_sp0828iicwritedata(0x3101, 0x6c);
	/*exposure gain*/
	Fingertech_sp0828iicwritedata(0x3102, 0x00);
	
	/*something done*/
	Fingertech_sp0828iicwritedata(0x3405, 0x00);
  Fingertech_sp0828iicwritedata(0x301d, 0x00);
	
	//Fingertech_sp0828iicwritedata(0x300a, 0x03);
	//Fingertech_sp0828iicwritedata(0x4027, 0x00);
	//Fingertech_sp0828iicwritedata(0x401f, 0x10);
	//Fingertech_sp0828iicwritedata(0x4006, 0x0b);
	//Fingertech_sp0828iicreaddata(0x4027, &chipid1);
	
	//work
	Fingertech_sp0828iicwritedata(0x3007, 0x00);
	
	while(chipid1 != 0x02){
		Fingertech_sp0828iicreaddata(0x3013,&chipid1);
		Fingertech_sp0828iicreaddata(0x301a,&chipid2);
		Fingertech_sp0828iicreaddata(0x3200,&chipid3);
		Fingertech_sp0828iicreaddata(0x3201,&chipid4);
		//printf("chipid=0x%02x,chipid=0x%02x,chipid=0x%02x,chipid=0x%02x\n",chipid1,chipid2,chipid3,chipid4);
	}
	CSI->CSICR18 = (CSI->CSICR18 & ~CSI_CSICR18_MASK_OPTION_MASK) | CSI_CSICR18_MASK_OPTION(0);
	CSI_SetRxBufferAddr(CSI, 0, (uint32_t)s_frameBuffer);
	CSI_SetRxBufferAddr(CSI, 1, (uint32_t)s_frameBuffer);
	CSI_EnableInterrupts(CSI, kCSI_RxBuffer1DmaDoneInterruptEnable | kCSI_RxBuffer0DmaDoneInterruptEnable);
	EnableIRQ(CSI_IRQn);
}

void M1_V1_Start(void)
{
	CSI_ReflashFifoDma(CSI, kCSI_RxFifo);
	CSI_Start(CSI);
	Fingertech_sp0828iicwritedata(0x301b, 0x01);
}

void M1_V1_Stop(void)
{

}

status_t DistinguishCamera(void)
{
	uint16_t ret = 0;
	
	ret = M1_V1_readid();
	if(ret == PIS1801){
		camera_cfg.stat=PIS1801;
		camera_cfg.exp_time=25;
		camera_cfg.x=430;
		camera_cfg.y=200;
		camera_cfg.width=224;
		camera_cfg.hight=224;
		camera_cfg.full_size_w=1000;
		camera_cfg.update=0;
		camera_cfg.transfer_w = 224;
		camera_cfg.transfer_h = 224;
		camera_cfg.x1 = 0;
		camera_cfg.x2 = 0;
		camera_cfg.x3 = 0;
	}else if(ret == PIS1701){
		camera_cfg.stat=PIS1701;
		camera_cfg.exp_time=20;
		camera_cfg.x=6;
		camera_cfg.y=470;
		camera_cfg.width=1920;
		camera_cfg.hight=200;
		camera_cfg.full_size_w=2000;
		camera_cfg.update=0;
		camera_cfg.transfer_w = W;
		camera_cfg.transfer_h = H;
		camera_cfg.x1 = 116;
		camera_cfg.x2 = 939;
		camera_cfg.x3 = 1765;
	}else{
		return -1;
	}
	
	FingerTech_FlashConfig();
	
	if(camera_cfg.stat == PIS1801){
		camera_ptr.init=M1_V1_Init;
		camera_ptr.start=M1_V1_Start;
		camera_ptr.stop=M1_V1_Stop;
	}else if(camera_cfg.stat == PIS1701){
		camera_ptr.init=PIS1701_Init;
		camera_ptr.start=PIS1701_Start;
		camera_ptr.stop=PIS1701_Stop;
	}

	return 0;
}

void Fingertech_cameraconfig(void)
{
	status_t status = 0;
	status = DistinguishCamera();
	if(status != 0){
		DEBUG("Can't find device");
		return;
	}
	
	const camera_config_t cameraConfig = {
			.pixelFormat = kVIDEO_PixelFormatYUYV,//kVIDEO_PixelFormatRGB565,//
			.bytesPerPixel = 1,/* rgb*/
			.resolution = FSL_VIDEO_RESOLUTION(camera_cfg.width, camera_cfg.hight),
			.frameBufferLinePitch_Bytes = camera_cfg.width,
			.interface = kCAMERA_InterfaceGatedClock,//kCAMERA_InterfaceCCIR656,//kCAMERA_InterfaceGatedClock,
			.controlFlags = (kCAMERA_HrefActiveHigh | kCAMERA_DataLatchOnRisingEdge),//(kCAMERA_HrefActiveLow | kCAMERA_DataLatchOnRisingEdge | kCAMERA_VsyncActiveLow),
			.framePerSec = 50,
	};
	
	Fingertech_CSI_Init(&cameraConfig);
		
	BOARD_PullCameraPowerDownPin(true);
		
	/* Delay 1ms. */
	M1_V1_DelayMs(1);
	
	BOARD_PullCameraPowerDownPin(false);
	
	camera_ptr.init();
}