/********************************************************************************
*工 程 名:屏下指纹项目
*工程功能:配置项目各模块底层驱动
*时    间:2018.12.4
*版    本:v0.1
********************************************************************************/

#include "main.h"
#include "gd32f4xx_pmu.h"
#include <stdint.h>
#include "gd32f4xx_rcu.h"
#include "common.h"
#include "usbd_msc_core.h"
#include "usb.h"
#include "usb_regs.h"
#include "usbd_conf.h"

extern usb_core_handle_struct usbhs_core_dev;

uint32_t size = 0;
uint32_t num = 0;
uint32_t count = 0;
uint32_t once_len = 1024;
uint8_t *p = NULL;

int main(void)
{
	nvic_vector_table_set(0x8004000,0);
    FingerTech_BspInit();		
		
    while(1){
		if(USB_RX_FLAG){
			SystemInit();
			USB_RX_FLAG = FALSE;
			size = usb_command_parse();

			count = size / once_len;
			usbd_ep_fifo_flush(&usbhs_core_dev, MSC_IN_EP);
			if(count == 0){
				usbd_ep_tx(&usbhs_core_dev, MSC_IN_EP, (uint8_t *)usb_tx, size);
			}else{
				usbd_ep_tx(&usbhs_core_dev, MSC_IN_EP, (uint8_t *)usb_tx, once_len);
			}
		}else if(USB_TX_FLAG){
			USB_TX_FLAG = FALSE;
			num++;
			p = usb_tx + num*once_len;
			if(num == count){
				once_len = size%once_len;
				if(once_len == 0){
					 usbd_ep_fifo_flush(&usbhs_core_dev, MSC_OUT_EP);
					 usbd_ep_rx (&usbhs_core_dev,MSC_OUT_EP,(uint8_t *)usb_rx, USB_COMMAND_SIZE);
					 system_clock_16m_irc16m();
					
					 continue;
				}
			}else if(num > count){
				num = 0;
				once_len = 1024;
				usbd_ep_fifo_flush(&usbhs_core_dev, MSC_OUT_EP);
				usbd_ep_rx (&usbhs_core_dev,MSC_OUT_EP,(uint8_t *)usb_rx, USB_COMMAND_SIZE);
				system_clock_16m_irc16m();
				
				continue;
			}
				usbd_ep_fifo_flush(&usbhs_core_dev, MSC_IN_EP);
				usbd_ep_tx (&usbhs_core_dev, MSC_IN_EP, p, once_len);
		}	
	}
		
	return 0;
} 
