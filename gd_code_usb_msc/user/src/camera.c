/********************************************************************************
*文 件 名：camera
*文件功能：将关于摄像头的初始化和功能接口集中于此
*版    本：v0.1
*时    间：2018.11.15
********************************************************************************/
#include "gd32f4xx_rcu.h"
#include "common.h"
#include "camera.h"

void camera_reset_pin_init(void)
{
		cdfigner_gpio_set(RCU_GPIOA, GPIOA, GPIO_PIN_4);
}