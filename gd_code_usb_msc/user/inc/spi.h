#ifndef SPI_H_
#define SPI_H_

void spi_cs_high(void);
void spi_cs_low(void);
void spi_gpio_config(void);
void spi_send_data(uint8_t *tx, uint8_t *rx, uint32_t len);

#endif
