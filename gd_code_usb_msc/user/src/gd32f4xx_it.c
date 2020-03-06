/*!
    \file  gd32f4xx_it.c
    \brief interrupt service routines
*/

/*
    Copyright (C) 2016 GigaDevice

    2016-08-15, V1.0.0, firmware for GD32F4xx
*/

#include "usbd_int.h"
#include "spi.h"
#include "systick.h"
#include "gd32f4xx_it.h"

bool rec_int_flag = FALSE;
bool spi_int_flag = FALSE;

extern usb_core_handle_struct usbhs_core_dev;

extern void usb_timer_irq (void);
extern void system_clock_config(void);

/*!
    \brief      this function handles SysTick exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void SysTick_Handler(void)
{
    delay_decrement();
}

void EXTI3_IRQHandler(void)
{
	spi_int_flag = TRUE;
	exti_interrupt_flag_clear(EXTI_3);
}

/********************************************************************************
*函 数 名：DMA0_Channel3_IRQHandler
*函数功能：spi1接收中断
*输    入：
*输    出：
*备    注：
********************************************************************************/
void DMA0_Channel3_IRQHandler(void){
	dma_interrupt_disable(DMA0,DMA_CH3,DMA_CHXCTL_FTFIE);
	dma_interrupt_flag_clear(DMA0,DMA_CH3,DMA_CHXCTL_FTFIE);
	spi_cs_high();
	rec_int_flag = TRUE;
}

/********************************************************************************
*函 数 名：DMA0_Channel4_IRQHandler
*函数功能：spi1发送中断
*输    入：
*输    出：
*备    注：
********************************************************************************/
void DMA0_Channel4_IRQHandler(void)
{
	dma_interrupt_disable(DMA0,DMA_CH4,DMA_CHXCTL_FTFIE);
	dma_interrupt_flag_clear(DMA0,DMA_CH4,DMA_CHXCTL_FTFIE);
//	spi_cs_high();
//	rec_int_flag = TRUE;
}


/*!
    \brief      this function handles Timer2 Handler
    \param[in]  none
    \param[out] none
    \retval     none
*/
void TIMER2_IRQHandler(void)
{
    usb_timer_irq();
}

#ifdef USE_USBFS

/*!
    \brief      this function handles USBFS wakeup interrupt handler
    \param[in]  none
    \param[out] none
    \retval     none
*/
void USBFS_WKUP_IRQHandler(void)
{
    if (usbhs_core_dev.cfg.low_power) {
        SystemInit();

        system_clock_config();

        rcu_pll48m_clock_config(RCU_PLL48MSRC_PLLQ);

        rcu_ck48m_clock_config(RCU_CK48MSRC_PLL48M);

        rcu_periph_clock_enable(RCU_USBFS);

        usb_clock_ungate(&usbhs_core_dev);
    }

    exti_interrupt_flag_clear(EXTI_18);
}

#elif defined(USE_USBHS)

/*!
    \brief      this function handles USBHS wakeup interrupt handler
    \param[in]  none
    \param[out] none
    \retval     none
*/
void USBHS_WKUP_IRQHandler(void)
{
    if (usbhs_core_dev.cfg.low_power) {
        SystemInit();

#ifdef USE_EMBEDDED_PHY
        rcu_pll48m_clock_config(RCU_PLL48MSRC_PLLQ);

        rcu_ck48m_clock_config(RCU_CK48MSRC_PLL48M);
#elif defined(USE_ULPI_PHY)
        rcu_periph_clock_enable(RCU_USBHSULPI);
#endif

        rcu_periph_clock_enable(RCU_USBHS);

        usb_clock_ungate(&usbhs_core_dev);
    }

    exti_interrupt_flag_clear(EXTI_20);
}

#endif /* USE_USBFS */

#ifdef USE_USBFS

/*!
    \brief      this function handles USBFS IRQ Handler
    \param[in]  none
    \param[out] none
    \retval     none
*/
void USBFS_IRQHandler(void)
{
    usbd_isr (&usbhs_core_dev);
}

#elif defined(USE_USBHS)

/*!
    \brief      this function handles USBHS IRQ Handler
    \param[in]  none
    \param[out] none
    \retval     none
*/
void USBHS_IRQHandler(void)
{
    usbd_isr (&usbhs_core_dev);
}

#endif /* USE_USBFS */

#ifdef USBHS_DEDICATED_EP1_ENABLED

/*!
    \brief      dedicated IN endpoint1 ISR handler
    \param[in]  none
    \param[out] none
    \retval     none
*/
void USBHS_EP1_In_IRQHandler(void)
{
    USBD_EP1IN_ISR_Handler (&usbhs_core_dev);
}

/*!
    \brief      dedicated OUT endpoint1 ISR handler
    \param[in]  none
    \param[out] none
    \retval     none
*/
void USBHS_EP1_Out_IRQHandler(void)
{
    USBD_EP1OUT_ISR_Handler (&usbhs_core_dev);
}

#endif /* USBHS_DEDICATED_EP1_ENABLED */
