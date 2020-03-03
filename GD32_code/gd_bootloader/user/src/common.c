/********************************************************************************
*文 件 名：common
*文件功能：负责各种外设的项目配置
*版    本：v0.1
*时    间：2018.11.15
********************************************************************************/
#include "main.h"

//printf 接口
#if 1
int fputc(int ch, FILE *f)
{
    usart_data_transmit(UART3, (uint8_t)ch);
    while(RESET == usart_flag_get(UART3, USART_FLAG_TBE));
    return ch;
}
#endif

/********************************************************************************
*函 数 名：cdfinger_usart_init
*函数功能：初始化串口通信接口
*输    入：
*输    出：
*备    注：
********************************************************************************/
void cdfinger_usart_init(void){
    gd_eval_com_init();  
} 

/********************************************************************************
*函 数 名：cdfinger_irqpin_init
*函数功能：初始化两个功能脚，一个中断触发引脚PD9
*输    入：
*输    出：
*备    注：引脚：PD9,触发方式：上升沿触发
********************************************************************************/
void cdfinger_irqpin_init(void){
    rcu_periph_clock_enable(RCU_GPIOC);
    gpio_mode_set(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_3);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3);
    gpio_bit_reset(GPIOC,GPIO_PIN_3);
}

//计算校验码
uint8_t cdfinger_calccheckcode(uint8_t* buf,uint32_t len)
{
    uint32_t count=0;
	uint8_t sum=0;
	for(count=0;count<len;count++)
	{
       sum+=buf[count];
	}
	return 0xff-sum;
}
//计算需要擦除的块的个数
uint8_t cdfinger_calcearsenum(uint32_t addr)
{
  uint8_t num=0;
  if((addr>=ADDR_FMC_SECTOR_1)&&(addr<ADDR_FMC_SECTOR_2))
  {
	num = 1;
  }
  else if((addr>=ADDR_FMC_SECTOR_2)&&(addr<ADDR_FMC_SECTOR_3))
  {
	 num = 2;
  }
  else if((addr>=ADDR_FMC_SECTOR_3)&&(addr<ADDR_FMC_SECTOR_4))
  {
	 num = 3;
  }
  else if((addr>=ADDR_FMC_SECTOR_4)&&(addr<ADDR_FMC_SECTOR_5))
  {
	 num = 4;
  }
  else if((addr>=ADDR_FMC_SECTOR_5)&&(addr<ADDR_FMC_SECTOR_6))
  {
	 num = 5;
  }
  else if((addr>=ADDR_FMC_SECTOR_6)&&(addr<ADDR_FMC_SECTOR_7))
  {
	 num = 6;
  }
  else if((addr>=ADDR_FMC_SECTOR_7)&&(addr<ADDR_FMC_SECTOR_8))
  {
	 num = 7;
  }
  return num;  
}

/********************************************************************************
*函 数 名：FingerTech_BspInit
*函数功能：按照项目需求初始化各种外设模块
*输    入：
*输    出：
*备    注：
********************************************************************************/
void cdfinger_bspinit(void){
//    systick_config();
    cdfinger_usart_init();
 //   cdfinger_irqpin_init();
}



