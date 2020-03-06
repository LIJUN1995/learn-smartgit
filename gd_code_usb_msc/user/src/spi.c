/********************************************************************************
*文 件 名：spi
*文件功能：将spi初始化和调用接口集中于此
*版    本：v0.1
*时    间：2018.11.15
********************************************************************************/
#include "gd32f4xx.h"
#include "gd32f4xx_it.h"
#include "spi.h"
#include <stdint.h>
#include "gd32f4xx_rcu.h"
#include "common.h"
#include "camera.h"

uint8_t SpiTxBuffer[SPI_TXRX_SIZE];
uint8_t SpiRxBuffer[SPI_TXRX_SIZE];

void spi_cs_low(void)
{
	GPIO_BC(GPIOB) = GPIO_PIN_12;
}

void spi_cs_high(void)
{
	GPIO_BOP(GPIOB) = GPIO_PIN_12;
}


void spi_gpio_config(void)
{    
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_DMA0);
    rcu_periph_clock_enable(RCU_SPI1);
    gpio_af_set(GPIOB, GPIO_AF_5, GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
	
		gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,GPIO_PIN_12);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_12);
	
		/*拉高cs*/
		spi_cs_high();
		
}

void spi_config(void)
{
    spi_parameter_struct spi_init_struct;
	
    /* chip select invalid */
    spi_i2s_deinit(SPI1);
    spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.device_mode          = SPI_MASTER;
    spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;
    spi_init_struct.nss                  = SPI_NSS_SOFT;
    spi_init_struct.prescale             = SPI_PSC_16;
    spi_init_struct.endian               = SPI_ENDIAN_MSB;
    spi_init(SPI1, &spi_init_struct);
	
    spi_dma_enable(SPI1, SPI_DMA_TRANSMIT);
    spi_dma_enable(SPI1, SPI_DMA_RECEIVE);
	
    nvic_irq_enable(DMA0_Channel4_IRQn, 3U, 0U);
    nvic_irq_enable(DMA0_Channel3_IRQn, 2U, 0U);
	
    spi_enable(SPI1);
}

void dma_tx_config(uint32_t arraysize,uint8_t * tx_addr)
{
    dma_single_data_parameter_struct dma_init_struct;
	
    dma_flag_clear(DMA0,DMA_CH4,DMA_INTF_HTFIF);
    dma_flag_clear(DMA0,DMA_CH4,DMA_INTF_FTFIF);
	
    /* SPI1 transmit dma config */
    dma_deinit(DMA0,DMA_CH4);
    dma_init_struct.periph_addr         = (uint32_t)&SPI_DATA(SPI1);
    dma_init_struct.memory0_addr        = (uint32_t)tx_addr;
    dma_init_struct.direction           = DMA_MEMORY_TO_PERIPH;
    dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
    dma_init_struct.priority            = DMA_PRIORITY_LOW;
    dma_init_struct.number              = arraysize;
    dma_init_struct.periph_inc          = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.memory_inc          = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.circular_mode       = DMA_CIRCULAR_MODE_DISABLE;
    dma_single_data_mode_init(DMA0,DMA_CH4,dma_init_struct);
    dma_channel_subperipheral_select(DMA0,DMA_CH4,DMA_SUBPERI0);
	
//		dma_interrupt_enable(DMA0,DMA_CH4,DMA_CHXCTL_HTFIE);
    dma_interrupt_enable(DMA0,DMA_CH4,DMA_CHXCTL_FTFIE);
    
    dma_channel_enable(DMA0,DMA_CH4);
}

void dma_rx_config(uint32_t arraysize,uint8_t * rx_addr,uint32_t memory_inc)
{
    dma_single_data_parameter_struct dma_init_struct;
	
    dma_flag_clear(DMA0,DMA_CH3,DMA_INTF_HTFIF);
    dma_flag_clear(DMA0,DMA_CH3,DMA_INTF_FTFIF);
    
    /* SPI0 receive dma config */
    dma_deinit(DMA0,DMA_CH3);
    dma_init_struct.periph_addr         = (uint32_t)&SPI_DATA(SPI1);
    dma_init_struct.memory0_addr        = (uint32_t)rx_addr;
    dma_init_struct.direction           = DMA_PERIPH_TO_MEMORY;
    dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
    dma_init_struct.priority            = DMA_PRIORITY_LOW;
    dma_init_struct.number              = arraysize;
    dma_init_struct.periph_inc          = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.memory_inc          = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.circular_mode       = DMA_CIRCULAR_MODE_DISABLE;
    dma_single_data_mode_init(DMA0,DMA_CH3,dma_init_struct);
    dma_channel_subperipheral_select(DMA0,DMA_CH3,DMA_SUBPERI0);
    
//    dma_interrupt_enable(DMA0,DMA_CH3,DMA_CHXCTL_HTFIE);
    dma_interrupt_enable(DMA0,DMA_CH3,DMA_CHXCTL_FTFIE);
	
    dma_channel_enable(DMA0,DMA_CH3);
}

void spi_send_data(uint8_t *tx, uint8_t *rx, uint32_t len)
{
	spi_config();
	spi_cs_low();
	dma_tx_config(len,tx);
  dma_rx_config(len,rx,DMA_MEMORY_INCREASE_ENABLE);
	
	/*等待DMA数据接收完成*/
	while(!rec_int_flag);
	rec_int_flag = FALSE;
}
