#include "main.h"
#include "FingerTech_common.h"
#include "FingerTech_camera.h"

static void PIS1701__full_frame_size(uint32_t height,uint32_t width)
{
    /*config column size*/
    Fingertech_sp0828iicwritedata(0x3200, height >> 8);
    Fingertech_sp0828iicwritedata(0x3201, height & 0x00ff);

    /*config row size*/
    Fingertech_sp0828iicwritedata(0x3202, width >> 8);
    Fingertech_sp0828iicwritedata(0x3203, width & 0x00ff);
}

static void PIS1701_out_frame_size(uint32_t height_base,uint32_t width_base,uint32_t height,uint32_t width)
{
	height = height - 1;
	width = width - 1;
	
	Fingertech_sp0828iicwritedata(0X3204, height_base >> 8);
	Fingertech_sp0828iicwritedata(0X3205, height_base&0x00ff);
	
	Fingertech_sp0828iicwritedata(0X3206, (height_base+height) >> 8);
	Fingertech_sp0828iicwritedata(0X3207, (height_base+height)&0x00ff);
	
	Fingertech_sp0828iicwritedata(0X3208, width_base >> 8);
	Fingertech_sp0828iicwritedata(0X3209, width_base&0x00ff);
	
	Fingertech_sp0828iicwritedata(0X320A, (width_base+width) >> 8);
	Fingertech_sp0828iicwritedata(0X320B, (width_base+width)&0x00ff);
}

static void PIS1701_change_exposureTime(int i)
{
	uint8_t np = 0,nb = 0;
	uint32_t temp = i*66000/camera_cfg.full_size_w;
	np=temp>>8&0xff;
	nb=(uint8_t)temp&0xff;
	//np = ((slaveRxData[2]*36000)/1650)/256;
	//nb = ((slaveRxData[2]*36000)/1650)%256;
	printf("np = 0x%02x,nb = 0x%02x\n",np,nb);
 if(temp <= camera_cfg.y+camera_cfg.hight)
		temp=camera_cfg.y+camera_cfg.hight;
	PIS1701__full_frame_size(temp+1,camera_cfg.full_size_w);

	Fingertech_sp0828iicwritedata(0x3100,np);
	Fingertech_sp0828iicwritedata(0x3101,nb);

}

static void PIS1701_config_exposureGain(uint8_t e_gain)
{
	Fingertech_sp0828iicwritedata(0x3102, e_gain);
}
void PIS1701_Init(void)
{
	Fingertech_sp0828iicwritedata(0x3006,0x06); // TM mode,power down
	Fingertech_sp0828iicwritedata(0x3009,0x00);  // Drive
	
	PIS1701_change_exposureTime(camera_cfg.exp_time);
	PIS1701_out_frame_size(camera_cfg.y,camera_cfg.x,camera_cfg.hight,camera_cfg.width);
	
	Fingertech_sp0828iicwritedata(0x3300,0x42);  // FBDIV_PLL
	Fingertech_sp0828iicwritedata(0x3301,0x00);  // REFDIV_PLL
	
	Fingertech_sp0828iicwritedata(0x3302,0x04);  // FRANGE1
	Fingertech_sp0828iicwritedata(0x3303,0x08);  // FRANGE2
	Fingertech_sp0828iicwritedata(0x3400,0x03);  // DVP
	Fingertech_sp0828iicwritedata(0x3700,0xD5);  // BLC,direct
	Fingertech_sp0828iicwritedata(0x3800,0x04);  // Digital RNC on
	Fingertech_sp0828iicwritedata(0x3B00,0x01);  // -n --> 0
	Fingertech_sp0828iicwritedata(0x3D00,0x01);  // TEMP SENSOR on
	Fingertech_sp0828iicwritedata(0x411B,0x00);  // Pixel????5.4u
	Fingertech_sp0828iicwritedata(0x4117,0x0F);  // Pixel?????
	Fingertech_sp0828iicwritedata(0x4131,0x00);  // VDARK_HIGH,??
	Fingertech_sp0828iicwritedata(0x3006,0x04);   // ??WORK
	
	//FingerTech_config_exposureTime(camera_cfg.exp_time);
	
	Fingertech_sp0828iicwritedata(0x3103,0x00); // PGA
	Fingertech_sp0828iicwritedata(0x3104,0x00); // ADC
	
	Fingertech_sp0828iicwritedata(0x310F,0x00);  // DCG
	
	CSI->CSICR18 = (CSI->CSICR18 & ~CSI_CSICR18_MASK_OPTION_MASK) | CSI_CSICR18_MASK_OPTION(0);
	CSI_SetRxBufferAddr(CSI, 0, (uint32_t)s_frameBuffer);
	CSI_SetRxBufferAddr(CSI, 1, (uint32_t)s_frameBuffer);
	CSI_EnableInterrupts(CSI, kCSI_RxBuffer1DmaDoneInterruptEnable | kCSI_RxBuffer0DmaDoneInterruptEnable);
	EnableIRQ(CSI_IRQn);
}
void PIS1701_Start(void)
{
		BOARD_PullCameraPowerDownPin(false);
		CSI_ReflashFifoDma(CSI, kCSI_RxFifo);
		CSI_Start(CSI);
}

void PIS1701_Stop(void)
{
		BOARD_PullCameraPowerDownPin(true);
}