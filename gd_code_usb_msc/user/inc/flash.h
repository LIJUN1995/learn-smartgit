#ifndef FLASH_H_
#define FLASH_H_

#include <stdio.h>
#include "gd32f4xx.h"
#include "stdint.h"
#include "camera.h"
/* base address of the FMC sectors */
#define ADDR_FMC_SECTOR_0     ((uint32_t)0x08000000) /*!< base address of sector 0, 16 kbytes */
#define ADDR_FMC_SECTOR_1     ((uint32_t)0x08004000) /*!< base address of sector 1, 16 kbytes */
#define ADDR_FMC_SECTOR_2     ((uint32_t)0x08008000) /*!< base address of sector 2, 16 kbytes */
#define ADDR_FMC_SECTOR_3     ((uint32_t)0x0800C000) /*!< base address of sector 3, 16 kbytes */
#define ADDR_FMC_SECTOR_4     ((uint32_t)0x08010000) /*!< base address of sector 4, 64 kbytes */
#define ADDR_FMC_SECTOR_5     ((uint32_t)0x08020000) /*!< base address of sector 5, 64 kbytes */
#define ADDR_FMC_SECTOR_6     ((uint32_t)0x08040000) /*!< base address of sector 6, 64 kbytes */
#define ADDR_FMC_SECTOR_7     ((uint32_t)0x08060000) /*!< base address of sector 7, 64 kbytes */
#define ADDR_FMC_SECTOR_8     ((uint32_t)0x08080000) /*!< base address of sector 8, 64 kbytes */
#define ADDR_FMC_SECTOR_9     ((uint32_t)0x080A0000) /*!< base address of sector 9, 64 kbytes */
#define ADDR_FMC_SECTOR_10    ((uint32_t)0x080C0000) /*!< base address of sector 10, 64 kbytes */
#define ADDR_FMC_SECTOR_11    ((uint32_t)0x080E0000) /*!< base address of sector 11, 64 kbytes */

#define FLASH_USER_START_ADDR ADDR_FMC_SECTOR_6
extern uint32_t size_w;

/* gets the sector of a given address */
uint32_t fmc_sector_get(uint32_t address);
/* erases the sector of a given sector number */
void fmc_erase_sector(uint32_t fmc_sector);
/* write 32 bit length data to a given address */
void fmc_write_32bit_data(uint32_t address, uint16_t length, int32_t* data_32);
/* read 32 bit length data from a given address */
void fmc_read_32bit_data(uint32_t address, uint16_t length, int32_t* data_32);
void fmc_write_8bit_data(uint32_t address, uint16_t length, int8_t* data_8);
void fmc_read_8bit_data(uint32_t address, uint16_t length, int8_t* data_8);
void fmc_write_16bit_data(uint32_t address, uint16_t length, int16_t* data_16);
void fmc_read_16bit_data(uint32_t address, uint16_t length, int16_t* data_16);
void FingerTech_FlashConfig(void);
void update_camera_cfg(void);
#endif
