#include <string.h>
#include "gd32f4xx_rcu.h"
#include "usb_std.h"
#include "usbd_conf.h"
#include "usb_regs.h"
#include "usb_core.h"
#include "usb.h"
#include "common.h"
#include "usbd_msc_core.h"
#include "usb_delay.h"
#include "spi.h"
#include "usbd_core.h"
#include "camera.h"

/*!
    \brief      configure system clock
    \param[in]  none
    \param[out] none
    \retval     none
*/
void system_clock_config(void)
{
    /* select PLL as system clock */
    RCU_CFG0 &= ~RCU_CFG0_SCS;
    RCU_CFG0 |= RCU_CKSYSSRC_HXTAL;

    /* wait until PLL is selected as system clock */
    while (0U == (RCU_CFG0 & RCU_SCSS_HXTAL)) {
    }
    
    rcu_osci_off(RCU_PLL_CK);

    RCU_PLL = (25U | (384U << 6U) | (((2U >> 1U) - 1U) << 16U) |
               (RCU_PLLSRC_HXTAL) | (8U << 24U));

    /* enable PLL */
    RCU_CTL |= RCU_CTL_PLLEN;

    /* wait until PLL is stable */
    while (0U == (RCU_CTL & RCU_CTL_PLLSTB)) {
    }

    /* select PLL as system clock */
    RCU_CFG0 &= ~RCU_CFG0_SCS;
    RCU_CFG0 |= RCU_CKSYSSRC_PLLP;

    /* wait until PLL is selected as system clock */
    while (0U == (RCU_CFG0 & RCU_SCSS_PLLP)) {
    }
}

/*!
    \brief      configure USB data line gpio
    \param[in]  none
    \param[out] none
    \retval     none
*/
void usb_gpio_config(void)
{
    rcu_periph_clock_enable(RCU_SYSCFG);

#ifdef USE_USBFS

    rcu_periph_clock_enable(RCU_GPIOA);

    /* USBFS_DM(PA11) and USBFS_DP(PA12) GPIO pin configuration */
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_11 | GPIO_PIN_12);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_200MHZ, GPIO_PIN_11 | GPIO_PIN_12);

    gpio_af_set(GPIOA, GPIO_AF_10, GPIO_PIN_11 | GPIO_PIN_12);

#elif defined(USE_USBHS)

    #ifdef USE_ULPI_PHY
        rcu_periph_clock_enable(RCU_GPIOA);
        rcu_periph_clock_enable(RCU_GPIOB);
        rcu_periph_clock_enable(RCU_GPIOC);
        rcu_periph_clock_enable(RCU_GPIOH);
        rcu_periph_clock_enable(RCU_GPIOI);

        /* ULPI_STP(PC0) GPIO pin configuration */
        gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_0);
        gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_200MHZ, GPIO_PIN_0);

        /* ULPI_CK(PA5) GPIO pin configuration */
        gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_5);
        gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_200MHZ, GPIO_PIN_5);

        /* ULPI_NXT(PH4) GPIO pin configuration */
        gpio_mode_set(GPIOH, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_4);
        gpio_output_options_set(GPIOH, GPIO_OTYPE_PP, GPIO_OSPEED_200MHZ, GPIO_PIN_4);

        /* ULPI_DIR(PI11) GPIO pin configuration */
        gpio_mode_set(GPIOI, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_11);
        gpio_output_options_set(GPIOI, GPIO_OTYPE_PP, GPIO_OSPEED_200MHZ, GPIO_PIN_11);

        /* ULPI_D1(PB0), ULPI_D2(PB1), ULPI_D3(PB10), ULPI_D4(PB11) \
           ULPI_D5(PB12), ULPI_D6(PB13) and ULPI_D7(PB5) GPIO pin configuration */
        gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, \
                        GPIO_PIN_5 | GPIO_PIN_13 | GPIO_PIN_12 |\
                        GPIO_PIN_11 | GPIO_PIN_10 | GPIO_PIN_1 | GPIO_PIN_0);
        gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_200MHZ, \
                        GPIO_PIN_5 | GPIO_PIN_13 | GPIO_PIN_12 |\
                        GPIO_PIN_11 | GPIO_PIN_10 | GPIO_PIN_1 | GPIO_PIN_0);

        /* ULPI_D0(PA3) GPIO pin configuration */
        gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_3);
        gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_200MHZ, GPIO_PIN_3);

        gpio_af_set(GPIOC, GPIO_AF_10, GPIO_PIN_0);
        gpio_af_set(GPIOH, GPIO_AF_10, GPIO_PIN_4);
        gpio_af_set(GPIOI, GPIO_AF_10, GPIO_PIN_11);
        gpio_af_set(GPIOA, GPIO_AF_10, GPIO_PIN_5 | GPIO_PIN_3);
        gpio_af_set(GPIOB, GPIO_AF_10, GPIO_PIN_5 | GPIO_PIN_13 | GPIO_PIN_12 |\
                                       GPIO_PIN_11 | GPIO_PIN_10 | GPIO_PIN_1 | GPIO_PIN_0);
    #endif /* USE_ULPI_PHY */

#endif /* USE_USBFS */
}

/*!
    \brief      configure USB clock
    \param[in]  none
    \param[out] none
    \retval     none
*/
void usb_clock_config(void)
{
#ifdef USE_USBFS
//    rcu_pll48m_clock_config(RCU_PLL48MSRC_PLLQ);
	
	RCU_ADDCTL |=(1<<16);
	while((RCU_ADDCTL & (0x1<<17))==0){}

    rcu_ck48m_clock_config(RCU_CK48MSRC_IRC48M);

    rcu_periph_clock_enable(RCU_USBFS);

#elif defined(USE_USBHS)

    rcu_periph_clock_enable(RCU_USBHS);

    #ifdef USE_EMBEDDED_PHY
        rcu_pll48m_clock_config(RCU_PLL48MSRC_PLLQ);

        rcu_ck48m_clock_config(RCU_CK48MSRC_PLL48M);

    #elif defined(USE_ULPI_PHY)
        rcu_periph_clock_enable(RCU_USBHSULPI);

    #endif /* USE_EMBEDDED_PHY */

#endif /* USB_USBFS */
}

/*!
    \brief      configure USB interrupt
    \param[in]  none
    \param[out] none
    \retval     none
*/
void usb_interrupt_config(void)
{
    nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);

#ifdef USE_USBFS
    nvic_irq_enable((uint8_t)USBFS_IRQn, 2U, 0U);
#elif defined(USE_USBHS)
    nvic_irq_enable((uint8_t)USBHS_IRQn, 2U, 0U);
#endif /* USE_USBFS */

#ifdef USBFS_LOW_PWR_MGMT_SUPPORT

    /* enable the power module clock */
    rcu_periph_clock_enable(RCU_PMU);

    /* USB wakeup EXTI line configuration */
    exti_interrupt_flag_clear(EXTI_18);
    exti_init(EXTI_18, EXTI_INTERRUPT, EXTI_TRIG_RISING);
    exti_interrupt_enable(EXTI_18);

    nvic_irq_enable((uint8_t)USBFS_WKUP_IRQn, 1U, 0U);

#elif defined(USBHS_LOW_PWR_MGMT_SUPPORT)

    /* enable the power module clock */
    rcu_periph_clock_enable(RCU_PMU);

    /* USB wakeup EXTI line configuration */
    exti_interrupt_flag_clear(EXTI_20);
    exti_init(EXTI_20, EXTI_INTERRUPT, EXTI_TRIG_RISING);
    exti_interrupt_enable(EXTI_20);

    nvic_irq_enable((uint8_t)USBHS_WKUP_IRQn, 1U, 0U);

#endif /* USBHS_LOW_PWR_MGMT_SUPPORT */

#ifdef USBHS_DEDICATED_EP1_ENABLED
    nvic_irq_enable(USBHS_EP1_Out_IRQn, 1, 0);
    nvic_irq_enable(USBHS_EP1_In_IRQn, 1, 0);
#endif /* USBHS_DEDICATED_EP1_ENABLED */
}

uint32_t usb_command_parse(void)
{
	 uint32_t *size = (uint32_t *)usb_rx;
	
	 nvic_irq_disable((uint8_t)USBFS_IRQn);
	 if(size[0]<=12){
			spi_send_data((usb_rx+4),usb_tx,size[0]);
	 }else{
			spi_send_data((usb_rx+4),usb_tx,size[0]);
	 }
	 nvic_irq_enable((uint8_t)USBFS_IRQn, 2U, 0U);
	 
	 return size[0];
}	

usb_core_handle_struct usbhs_core_dev =
{
    .dev = 
    {
        .dev_desc = (uint8_t *)&device_descripter,
        .config_desc = (uint8_t *)&configuration_descriptor,
        .strings = usbd_strings,
        .class_init = msc_init,
        .class_deinit = msc_deinit,
        .class_req_handler = msc_req_handler,
        .class_data_handler = msc_data_handler
    },

    .udelay = usb_udelay,
    .mdelay = usb_mdelay
};

void usb_init(void)
{
	/* configure the system clock to 168MHz */
//    system_clock_config();

    /* configure USB GPIO */
    usb_gpio_config();

    /* configure USB clock */
    usb_clock_config();

    /* USB timer configure */
    usb_timer_init();

    /* USB device stack configure */
    usbd_init(&usbhs_core_dev, 
#ifdef USE_USBFS
    USB_FS_CORE_ID
#elif defined(USE_USBHS)
    USB_HS_CORE_ID
#endif
    );

    /* USB interrupt configure */
    usb_interrupt_config();

    /* check if USB device is enumerated successfully */
    while (usbhs_core_dev.dev.status != USB_STATUS_CONFIGURED) {}
}
