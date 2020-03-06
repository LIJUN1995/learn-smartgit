/********************************************************************************
*文 件 名：common
*文件功能：负责各种外设的项目配置
*版    本：v0.1
*时    间：2018.11.15
********************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include "gd32f4xx_rcu.h"
#include "common.h"
#include "gd32f4xx_eval.h"
#include "gd32f4xx_it.h"
#include "usb.h"
#include "camera.h"
#include "spi.h"
#include "systick.h"

void system_clock_16m_irc16m(void)
{
    uint32_t timeout = 0U;
    uint32_t stab_flag = 0U;
    
    /* enable IRC16M */
    RCU_CTL |= RCU_CTL_IRC16MEN;
    
    /* wait until IRC16M is stable or the startup time is longer than IRC16M_STARTUP_TIMEOUT */
    do{
        timeout++;
        stab_flag = (RCU_CTL & RCU_CTL_IRC16MSTB);
    }while((0U == stab_flag) && (IRC16M_STARTUP_TIMEOUT != timeout));
    
    /* if fail */
    if(0U == (RCU_CTL & RCU_CTL_IRC16MSTB)){
        while(1){
        }
    }
    
    /* AHB = SYSCLK */
    RCU_CFG0 |= RCU_AHB_CKSYS_DIV1;
    /* APB2 = AHB */
    RCU_CFG0 |= RCU_APB2_CKAHB_DIV1;
    /* APB1 = AHB */
    RCU_CFG0 |= RCU_APB1_CKAHB_DIV1;
    
    /* select IRC16M as system clock */
    RCU_CFG0 &= ~RCU_CFG0_SCS;
    RCU_CFG0 |= RCU_CKSYSSRC_IRC16M;
    
    /* wait until IRC16M is selected as system clock */
    while(0 != (RCU_CFG0 & RCU_SCSS_IRC16M)){
    }
}

int fputc(int ch, FILE *f)
{
#ifdef TYPE_ONE
    usart_data_transmit(UART4, (uint8_t)ch);
    while(RESET == usart_flag_get(UART4, USART_FLAG_TBE));
#else
    usart_data_transmit(UART3, (uint8_t)ch);
    while(RESET == usart_flag_get(UART3, USART_FLAG_TBE));
#endif
    return ch;
}

void cdfigner_gpio_set(rcu_periph_enum periph, uint32_t gpio_periph, uint32_t pin)
{
    /* enable the led clock */
    rcu_periph_clock_enable(periph);
	
    /* configure led GPIO port */ 
    gpio_mode_set(gpio_periph, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,pin);
    gpio_output_options_set(gpio_periph, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,pin);
		
		/*pull down*/
    GPIO_BC(gpio_periph) = pin;
    
		/*pull up*/
    GPIO_BOP(gpio_periph) = pin;
}

void cdfinger_usart_init(void){
    gd_eval_com_init();  
#ifdef SPI_TRANSFER
    printf("\n\n\n\n\n\n\n");
    printf("uart init success!\n");
#endif
} 

void cdfinger_irqpin_init(void)
{
    rcu_periph_clock_enable(RCU_GPIOC);
		rcu_periph_clock_enable(RCU_SYSCFG);
	
     /* GPIO mode set */
    gpio_mode_set(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, GPIO_PIN_3);
    
    /* enable and set EXTI interrupt to the lowest priorty */
    nvic_irq_enable(EXTI3_IRQn, 2U, 0U);
    
    /* connect EXTI line to key GPIO pin */
    syscfg_exti_line_config(EXTI_SOURCE_GPIOC, EXTI_SOURCE_PIN3);
    
    /* configure key EXTI line */
    exti_init(EXTI_3, EXTI_INTERRUPT, EXTI_TRIG_RISING);
    exti_interrupt_flag_clear(EXTI_3);
	
    exti_event_enable(EXTI_0);
    exti_interrupt_enable(EXTI_3);
}

void cdfinger_exti_init(void)
{
	 /* enable the key clock */
	rcu_periph_clock_enable(RCU_SYSCFG);
	
	/* enable and set key EXTI interrupt to the lowest priority */
	nvic_irq_enable(EXTI3_IRQn, 2U, 0U);

	/* connect key EXTI line to key GPIO pin */
	syscfg_exti_line_config(EXTI_SOURCE_GPIOC, EXTI_SOURCE_PIN3);

	/* configure key EXTI line */
	exti_init(EXTI_3, EXTI_INTERRUPT, EXTI_TRIG_RISING);
	
	exti_interrupt_flag_clear(EXTI_3);
	exti_interrupt_enable(EXTI_3);
	
}

void wait_spi_irq(void)
{
	while(!spi_int_flag);
	spi_int_flag = FALSE;
}


void FingerTech_BspInit(void){
    systick_config();
    camera_reset_pin_init();
	spi_gpio_config();
	usb_init();
}