#ifndef COMMON_H_
#define COMMON_H_

#define BUFFER_SIZE    (240*240)
#define USB_COMMAND_SIZE    16
#define SPI_TXRX_SIZE  5

#define IMG_WIDTH      240
#define IMG_HEIGHT     160
#define RAW_IMAGE_SIZE (IMG_WIDTH*IMG_HEIGHT*12/8)

enum __usb_command{
	READ_REG = 0x72,
	WRITE_REG,
	READ_IMG,
};

void wait_spi_irq(void);
void FingerTech_BspInit(void);
void system_clock_16m_irc16m(void);
void cdfigner_gpio_set(rcu_periph_enum periph, uint32_t gpio_periph, uint32_t pin);

#endif
