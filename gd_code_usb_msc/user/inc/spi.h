#ifndef SPI_H_
#define SPI_H_

void spi_cs_high(void);
void spi_cs_low(void);
void spi_conf_init(void);
void spi_gpio_config(void);
uint8_t spi_read_register(uint16_t reg);
void spi_write_register(uint16_t reg, uint8_t value);
void spi_send_data(uint8_t *tx, uint8_t *rx, uint32_t len);

#endif
