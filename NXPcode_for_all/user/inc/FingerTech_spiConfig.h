#ifndef _FINGERTECH_SPICONFIG_H
#define _FINGERTECH_SPICONFIG_H

#define LPSPI_SLAVE_BASEADDR (LPSPI1)
#define LPSPI_SLAVE_IRQN (LPSPI1_IRQn)
#define LPSPI_SLAVE_IRQHandler LPSPI1_IRQHandler

#define LPSPI_SLAVE_PCS_FOR_INIT (kLPSPI_Pcs0)
#define LPSPI_SLAVE_PCS_FOR_TRANSFER (kLPSPI_SlavePcs0)

#define LP_DMA DMA0
#define LP_DMAMUX DMAMUX

#define SPI_TX_DMA_LPSPI_CHANNEL (0U)
#define SPI_RX_DMA_LPSPI_CHANNEL (1U)

#define RX_DMA_SOURCE (kDmaRequestMuxLPSPI1Rx)
#define TX_DMA_SOURCE (kDmaRequestMuxLPSPI1Tx)

/* Select USB1 PLL PFD0 (720 MHz) as lpspi clock source */
#define LPSPI_CLOCK_SOURCE_SELECT (1U)
/* Clock divider for master lpspi clock source */
#define LPSPI_CLOCK_SOURCE_DIVIDER (7U)

extern unsigned char flag;
extern volatile bool fg_command;

extern uint32_t activeFrameAddr;
extern uint32_t inactiveFrameAddr;
extern camera_device_handle_t cameraDevice;
extern camera_receiver_handle_t cameraReceiver;

extern void Fingertech_SpiConfig(void);
extern void FingerTech_change_exposureGain(void);
extern void FingerTech_change_exposureTime(int time);
extern int FingerTech_move_image(uint16_t *img,int height, int width);
extern void DMA_transfer_data(void *tx,void *rx,uint32_t length);
extern void EDMA_Callback(LPSPI_Type *base,lpspi_slave_edma_handle_t *handle,status_t status,void *userData);

#endif