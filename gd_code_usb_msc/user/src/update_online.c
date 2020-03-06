/********************************************************************************
*工 程 名:屏下指纹项目
*工程功能:配置项目各模块底层驱动
*时    间:2018.12.4
*版    本:v0.1
********************************************************************************/
#include "gd32f4xx_misc.h"
#include "gd32f4xx_dma.h"
#include <stdio.h>
#include "common.h"
#include "flash.h"
#include "spi.h"
extern uint8_t spi_send_done;

//计算校验和
uint8_t cdfinger_calccheckcode(uint8_t *buf,uint32_t len)
{
  uint8_t sum=0;
  uint32_t count=0;
  for(count=0;count<len;count++){
    sum+=buf[count];
  }
  return sum;
}

int update_fw(void)
{
  uint32_t appsize = 0;
  uint8_t checkcode = 0;
  uint8_t calccheckcode = 0;
  uint8_t erasenum=0;
  uint32_t erasecount = 0;
  uint32_t eraseaddr = 0;
  uint32_t eraseret = 0;
  uint16_t camera_cfg_for_flash[9];
  uint8_t updatafwbuf[7];

  //文件大小和校验码
  appsize = ((slaveRxData[1]<<24)|(slaveRxData[2]<<16)|(slaveRxData[3]<<8)|(slaveRxData[4]));
  checkcode = slaveRxData[5];
	
//  //擦除
//  appsize%(64*1024) == 0 ? (erasenum= (appsize/(64*1024))):(erasenum = (appsize/(64*1024)+1));
//  for(erasecount=0;erasecount<erasenum;erasecount++){
//    eraseaddr = 0x08080000 + erasecount*(64*1024);
//    eraseret = fmc_sector_get(eraseaddr);
//    fmc_erase_sector(eraseret);
//  }
//  ob_write_protection0_disable(OB_WP_ALL);
	
	spi_config();
	dma_tx_config(appsize,slaveTxData);
	dma_rx_config(appsize,(uint8_t *)picture_data,DMA_MEMORY_INCREASE_ENABLE);
	spi_send_done = 0;
	
  //接收数据,每次接收4k
  gpio_bit_set(GPIOC,GPIO_PIN_3);
  while(spi_send_done == 0);


	//擦除
  appsize%(64*1024) == 0 ? (erasenum= (appsize/(64*1024))):(erasenum = (appsize/(64*1024)+1));
  for(erasecount=0;erasecount<erasenum;erasecount++){
    eraseaddr = 0x08080000 + erasecount*(64*1024);
    eraseret = fmc_sector_get(eraseaddr);
    fmc_erase_sector(eraseret);
  }
  ob_write_protection0_disable(OB_WP_ALL);
	//写入数据
	fmc_write_8bit_data(eraseaddr,appsize,(uint8_t *)picture_data);
	
  //判断校验码
  if((0xff - cdfinger_calccheckcode((uint8_t *)picture_data,appsize)) != checkcode){
    //升级失败死机,硬件复位重置
    printf("crc faile\r\n");
    NVIC_SystemReset();
    gpio_bit_reset(GPIOC,GPIO_PIN_3);
  }
  
  //擦最后的一个扇区写入升级信息和摄像头参数
  fmc_erase_sector(CTL_SECTOR_NUMBER_11);
  ob_write_protection0_disable(OB_WP_11);
  updatafwbuf[0] = 0xa5;
  updatafwbuf[1] = 0x5a;
  updatafwbuf[2] = (uint8_t)(appsize>>24);
  updatafwbuf[3] = (uint8_t)(appsize>>16);
  updatafwbuf[4] = (uint8_t)(appsize>>8);
  updatafwbuf[5] = (uint8_t)appsize;
  updatafwbuf[6] = checkcode;
  //写升级信息
  fmc_write_8bit_data(0x080FFF00,7,updatafwbuf);
  fmc_read_8bit_data(0x080FFF00,7,slaveRxData);
	reset_device();
  //复位
  NVIC_SystemReset();
}

/*
int update_fw(void)
{
  uint32_t appsize = 0;
  uint8_t checkcode = 0;
  uint8_t calccheckcode = 0;
  uint8_t erasenum=0;
  uint32_t erasecount = 0;
  uint32_t eraseaddr = 0;
  uint32_t eraseret = 0;
  uint16_t camera_cfg_for_flash[9];
  uint8_t updatafwbuf[7];

  //文件大小和校验码
  appsize = ((slaveRxData[1]<<24)|(slaveRxData[2]<<16)|(slaveRxData[3]<<8)|(slaveRxData[4]));
  checkcode = slaveRxData[5];
  
  //读摄像头的参数
  fmc_read_32bit_data(FLASH_USER_START_ADDR, size_w, (uint32_t *)&camera_cfg);
	
  //擦除
  appsize%(64*1024) == 0 ? (erasenum= (appsize/(64*1024))):(erasenum = (appsize/(64*1024)+1));
  printf("erasenum = %d\r\n",erasenum);
  for(erasecount=0;erasecount<erasenum;erasecount++)
  {
    eraseaddr = 0x08080000 + erasecount*(64*1024);
    eraseret = fmc_sector_get(eraseaddr);
    fmc_erase_sector(eraseret);
  }
  ob_write_protection0_disable(OB_WP_ALL);

  //接收数据,每次接收4k
  gpio_bit_set(GPIOC,GPIO_PIN_3);
  delay_1ms(1);
  spi_send_done = 0;
  for(erasecount=0;erasecount<appsize;erasecount+=4096)
  {
    spi_config();
    dma_tx_config(4096,slaveTxData);
    dma_rx_config(4096,slaveRxData,DMA_MEMORY_INCREASE_ENABLE);
    while(spi_send_done == 0);
    spi_send_done = 0;

    //写入数据
    eraseaddr = 0x08080000 + erasecount;
    fmc_write_8bit_data(eraseaddr,4096,&slaveRxData[0]);
    if(erasecount==(appsize-(appsize%4096)))
    { 
     calccheckcode += cdfinger_calccheckcode(slaveRxData,(appsize%4096));
    }else{
     calccheckcode += cdfinger_calccheckcode(slaveRxData,4096);
    }
  }
	
  //判断校验码
  if((0xff - calccheckcode) != checkcode)
  {
    //升级失败死机,硬件复位重置
    printf("crc faile\r\n");
    NVIC_SystemReset();
    gpio_bit_reset(GPIOC,GPIO_PIN_3);
  }
  //拉低
  gpio_bit_reset(GPIOC,GPIO_PIN_3);
  delay_1ms(1);
  //擦最后的一个扇区写入升级信息和摄像头参数
  fmc_erase_sector(CTL_SECTOR_NUMBER_11);
  ob_write_protection0_disable(OB_WP_11);
  fmc_write_32bit_data(FLASH_USER_START_ADDR,size_w,(uint32_t *)&camera_cfg);
  updatafwbuf[0] = 0xa5;
  updatafwbuf[1] = 0x5a;
  updatafwbuf[2] = (uint8_t)(appsize>>24);
  updatafwbuf[3] = (uint8_t)(appsize>>16);
  updatafwbuf[4] = (uint8_t)(appsize>>8);
  updatafwbuf[5] = (uint8_t)appsize;
  updatafwbuf[6] = checkcode;
  //写升级信息
  fmc_write_8bit_data(0x080FFF00,7,updatafwbuf);
  fmc_read_8bit_data(0x080FFF00,7,slaveRxData);
//  delay_1ms(1);
  //复位
  NVIC_SystemReset();
}
*/