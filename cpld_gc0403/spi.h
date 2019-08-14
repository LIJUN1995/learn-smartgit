#ifndef SPI_H
#define SPI_H

#include <stdint.h>
#include "sys/ioctl.h"

typedef struct _cdfinger_spi_data {
	unsigned char *rx;
	unsigned char *tx;
	int length;
	int tx_length;
} cdfinger_spi_data;

char spi_send_data(cdfinger_spi_data *data);
void init_flash(void);
void erase_flash(uint8_t cmd,uint32_t addr);
void read_flash(uint8_t cmd,uint32_t addr,uint8_t *rd_data, int len);
void page_program(uint8_t cmd,uint32_t addr,uint8_t *pp_data, int len);
void page_program_size(uint32_t addr,uint8_t *pp_data, int len);
void ProgramUserCode(void);
void ProgramDoneBit(void);
void Bypass(void);
void ISC_Disable(void);
void Resfresh(void);
void ReadUserCode(void);
void SPI_Verify(void);
void WriteFeatureRow(void);

#endif
