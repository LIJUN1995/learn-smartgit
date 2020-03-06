/********************************************************************************
*文 件 名：camera
*文件功能：将关于摄像头的初始化和功能接口集中于此
*版    本：v0.1
*时    间：2018.11.15
********************************************************************************/
#include "gd32f4xx.h"
#include "spi.h"
#include "gd32f4xx_gpio.h"
#include <stdint.h>
#include "gd32f4xx_rcu.h"
#include "common.h"
#include "camera.h"

static unsigned char fusion_frame_config_value = 0;

uint8_t raw_img_tx[4];
uint8_t raw_img_rx[4];

static unsigned short SENSOR_INIT_CONF[][2] = {
    {0x0330, 0xff}, {0x0330, 0x00}, {0x0330, 0xff}, {0x0330, 0x00}, {0x0330, 0xff}, {0x0330, 0x00}, {0x0300, 0x07},
    {0x0301, 0x23}, {0x0304, 0xc6}, {0x0305, 0x01}, {0x0302, 0x28}, {0x0306, 0x90}, {0x02ee, 0x30}, {0x0108, 0x06},
    {0x0109, 0xa0}, {0x0232, 0xc4}, {0x02d1, 0xd1}, {0x02cf, 0xa3}, {0x0221, 0x06}, {0x0229, 0x60}, {0x02ce, 0x6c},
    {0x0244, 0x20}, {0x0120, 0x01}, {0x016f, 0x01}, {0x0171, 0x11}, {0x0160, 0x25}, {0x0161, 0x06}, {0x0162, 0x00},
    {0x0163, 0x00}, {0x0164, 0x08}, {0x0287, 0x18}, {0x0297, 0xa3}, {0x017c, 0x28}, {0x0242, 0x9e}, {0x0243, 0x27},
    {0x010a, 0x00}, {0x010b, 0x08}, {0x010c, 0x00}, {0x010d, 0x04}, {0x010e, 0x05}, {0x010f, 0xa0}, {0x0119, 0x11},
    {0x0112, 0xe0}, {0x0113, 0x40}, {0x0117, 0x01}, {0x0118, 0x3f}, {0x0060, 0x00}, {0x0059, 0x00}, {0x0202, 0x01},
    {0x0203, 0x20}, {0x0202, 0x07}, {0x0203, 0x60}, {0x02b3, 0x00}, {0x0089, 0x03}, {0x2b0, 0x38},
};

void camera_reset_pin_init(void)
{
		cdfigner_gpio_set(RCU_GPIOA, GPIOA, GPIO_PIN_4);
}

bool camera_verify_id(void) 
{
    uint8_t ret = 0;

    ret = spi_read_register(0x3f0);
    if (ret != 0x27) {
        return FALSE;
    }

    return TRUE;
} 


void camera_init(void)
{
		uint8_t ret = 0;
		int count = sizeof(SENSOR_INIT_CONF) / sizeof(SENSOR_INIT_CONF[0]);
		int i = 0;
    for (i = 0; i < count; ++i) {
        spi_write_register(SENSOR_INIT_CONF[i][0], (unsigned char)SENSOR_INIT_CONF[i][1]);   
    }
		
		 for (i = 0; i < count; ++i) {
       ret = spi_read_register(SENSOR_INIT_CONF[i][0]);   
    }
}

int sensor_setImgWH(int width_base, int height_base, int width, int height) {
    unsigned char temp = 0;

    spi_write_register(0x115, (unsigned char)width_base);
		spi_write_register(0x116, (unsigned char)height_base);

    temp = (width & 0x00000700) >> 4 | (height & 0x00000300) >> 8;
	
    spi_write_register(0x119, temp);
		spi_write_register(0x112, (unsigned char)width);
    spi_write_register(0x113, (unsigned char)height);
    spi_write_register(0x118, (unsigned char)height - 1);

    return 0;
}

int sensor_setBinning(int binning_mode) {
    unsigned char value = 0;

    switch (binning_mode) {
    case 2:
        value = 0x25;
        sensor_setImgWH(0, 0, 480, 320);
        break;
    case 3:
        value = 0x3a;
        sensor_setImgWH(0, 0, 480, 318);
        break;
    case 4:
        value = 0x4f;
        sensor_setImgWH(0, 0, 480, 320);
        break;
    default:
        return -1;
    }

    spi_write_register(0x160, value);

    return 0;
}

static int sensor_setExpoTime(int time) {
    unsigned char temp = 0;
    unsigned char ret = 0;
    float row_time = 33.5;
    unsigned short exp = ((int)((time * 1000 / 35.3) * 3 - 12)) / 3 * 3;

    if (exp > 0x3fff) {
        exp = 0x3fff;
    }

    temp = exp >> 8;
    spi_write_register(0x202, temp);

    temp = (unsigned char)exp;
    spi_write_register(0x203, temp);

    return 0;
}

static int sensor_setImgGain(unsigned char gain) {
    unsigned char ret = 0;
    unsigned char temp_gain = 0;

    switch (gain) {
    case 1:
        temp_gain = 0x00;
        break;
    case 2:
        temp_gain = 0x01;
        break;
    case 3:
        temp_gain = 0x02;
        break;
    case 4:
        temp_gain = 0x03;
        break;
    case 5:
        temp_gain = 0x04;
        break;
    case 6:
        temp_gain = 0x0c;
        break;
    case 7:
        temp_gain = 0x14;
        break;
    case 8:
        temp_gain = 0x24;
        break;
    case 9:
        temp_gain = 0x2d;
        break;
    default:
        break;
    }

    spi_write_register(0x2b3, temp_gain);

    return 0;
}

static int sensor_setFrameNum(uint32_t count) {
    unsigned char b = 0;

    switch (count) {
    case 1:
        fusion_frame_config_value = 0x00;
        b = 0x02;
        break;

    case 2:
        fusion_frame_config_value = 0x80;
        b = 0x03;
        break;

    case 4:
        fusion_frame_config_value = 0x81;
        b = 0x05;
        break;

    case 8:
        fusion_frame_config_value = 0x82;
        b = 0x09;
        break;

    case 16:
        fusion_frame_config_value = 0x83;
        b = 0x11;
        break;

    default:
        return -1;
    }

    spi_write_register(0x17b, fusion_frame_config_value);
    spi_write_register(0x121, b);

    return 0;
}

void camera_pre_image(uint8_t val)
{
	/*set interrupt trigger point of camera*/
	spi_write_register(0x171, val);
	
	/*clear camera fifo*/
	spi_write_register(0x330, 0x20);
	spi_write_register(0x330, 0x00);
	
	/*prepare image*/
	spi_write_register(0x12f, 0x01);
}

static struct m_bit {
    unsigned long a : 8;
    unsigned long b : 8;
    unsigned long c : 4;
    unsigned long d : 4;
} * m_bit_p;

static void convert_img(unsigned short *dst, unsigned char *src, int width, int height) {
    int len = width * height;
    int i = 0, j = 0;

    for (i = 0; i < len; i += 2) {
        m_bit_p = (struct m_bit *)(src + j * 3);
        dst[i] = ((m_bit_p->a << 4) | m_bit_p->d) & 0x0fff;
        dst[i + 1] = ((m_bit_p->b << 4) | m_bit_p->c) & 0x0fff;
        j++;
    }

    return;
}

void camera_get_image(void)
{
	uint32_t spi_len = RAW_IMAGE_SIZE+4;
	
	spi_write_register(0x17b, fusion_frame_config_value & 0x0f);
	spi_write_register(0x171, 0x04);
	
	raw_img_tx[0] = 0xf4;
	raw_img_tx[1] = (RAW_IMAGE_SIZE & 0xff00) >> 8;
	raw_img_tx[2] = (RAW_IMAGE_SIZE & 0x00ff);
	raw_img_tx[3] = 0x00;
	
	spi_send_data(raw_img_tx, raw_img_rx, spi_len);
	
	unsigned short *p = (unsigned short *)raw_img_tx;
	convert_img(p,raw_img_rx+4,IMG_WIDTH,IMG_HEIGHT);
	
	int i = 0;
	for(i=0;i<IMG_WIDTH*IMG_HEIGHT;i++)
		raw_img_rx[i] = p[i]>>4;
	
	spi_write_register(0x16f, 0x00);
	spi_write_register(0x17b, fusion_frame_config_value);
	
}