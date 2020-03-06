#ifndef _MAIN_H
#define _MAIN_H

#include <stdio.h>
#include <board.h>
#include <stdarg.h>
#include <fsl_csi.h>
#include <pin_mux.h>
#include <fsl_edma.h>
#include <fsl_dcdc.h>
#include <fsl_qtmr.h>
#include <fsl_cache.h>
#include <fsl_lpspi.h>
#include <fsl_lpi2c.h>
#include <fsl_SP0828.h>
#include <fsl_camera.h>
#include <fsl_dmamux.h>
#include <fsl_elcdif.h>
#include <fsl_iomuxc.h>
#include <fsl_lpuart.h>
#include <fsl_flexspi.h>
#include <clock_config.h>
#include <fsl_lpspi_edma.h>
#include <fsl_camera_device.h>
#include <fsl_debug_console.h>
#include <fsl_camera_receiver.h>
#include <fsl_device_registers.h>
#include <fsl_csi_camera_adapter.h>

//#define  CAMERA_8BIT
//#define  CAMERA_10BIT  
#define  CAMERA_12BIT

//#define P1701
#define P1801

#define INT_GPIO GPIO5
#define INT_GPIO_PIN (0U)
#define FINGERTECH_LED_GPIO						GPIO3
#define FINGERTECH_LED_GPIO_PIN					(4U)
#define FINGERTECH_LED_IOMUXC					IOMUXC_GPIO_SD_B1_04_GPIO3_IO04

#define TRANSFER_SIZE  224*224
#define TRANSFER_SIZE_O  1920*200

#define IMG_NUM 3

#define M_W 184
#define M_H 200

#define W (M_W*IMG_NUM)
#define H M_H

//#define X1 (116 - M_W/2)
//#define X2 (939 - M_W/2)
//#define X3 (1765 - M_W/2)


//#define __DEBUG__
#ifdef __DEBUG__
	#define DEBUG(format,...) printf("File: "__FILE__", Line: %05d: "format"\n", __LINE__, ##__VA_ARGS__)
#else  
	#define DEBUG(format,...)  
#endif 

#pragma pack(push)
#pragma pack(1)
typedef struct cdfinger_chip
{
  int x;
  int y;
	uint16_t stat;
	uint16_t width;
  uint16_t hight;
	uint16_t update;
	uint16_t exp_time;
	uint16_t transfer_w;
	uint16_t transfer_h;
	uint16_t camera_size;
	uint16_t full_size_w;
	uint16_t x1;
  uint16_t x2;
	uint16_t x3;
}chip_cfg;
#pragma pack(pop)

typedef struct cdfinger_ptr
{
	void(*init)(void);
	void(*start)(void);
	void(*stop)(void);
}chip_ptr;

enum chip_command{
    ZERO,            	
    READ_IMAGE  = 0x90,    	
    CHIP_RESET,      	
    CHIP_ID,		       	
    EXPOSURE_TIME,  	
    EXPOSURE_GAIN, 
	
    MV_IMG = 0x95,         	
    RESOLUTION,      	
    UPDATA_PROGRAM,  	
		SLEEP_MODE,      	
    DEEP_SLEEP_MODE, 
	
    STANDBY_MODE = 0x9a,    	
    SINGLE_FRAME,    	
		MULTI_FRAME,     	
		ENROLL,
		GET_ATTRIBUTE,
	
		SET_COORDINATES = 0x9f
};

enum chip_type{
	PIS1801 = 0x1801,
	PIS1701 = 0X1701,
};

#endif