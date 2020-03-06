/*!
    \file  usbd_sram_access.c
    \brief internal flash access driver
*/

/*
    Copyright (C) 2016 GigaDevice

    2016-10-19, V1.0.0, demo for GD32F4xx
*/

#include "usb_conf.h"
#include "usbd_sram_access.h"

unsigned char SRAM[160 * 1024];

/*!
    \brief      read data from multiple blocks of internal SRAM
    \param[in]  pbuf: pointer to user buffer
    \param[in]  read_addr: address to be read
    \param[in]  block_size: size of block
    \param[in]  block_num: number of block
    \param[out] none
    \retval     operation status
*/
uint32_t  sram_multi_blocks_read (uint8_t *pbuf,
                                  uint32_t read_addr,
                                  uint16_t block_size,
                                  uint32_t block_num)
{
    uint32_t i = 0, offset = 0;

    while (block_num--) {
        for (i = 0; i < block_size; i++) {
            *pbuf = SRAM[read_addr + offset + i];
            pbuf++;
        }

        offset += block_size;
    }

    return 0;
}

/*!
    \brief      write data from multiple blocks of internal SRAM
    \param[in]  pbuf: pointer to user buffer
    \param[in]  write_addr: address to be write
    \param[in]  block_size: size of block
    \param[in]  block_num: number of block
    \param[out] none
    \retval     operation status
*/
uint32_t sram_multi_blocks_write (uint8_t *pbuf,
                                  uint32_t write_addr,
                                  uint16_t block_size,
                                  uint32_t block_num)
{
    uint32_t i = 0, Offset = 0;

    while (block_num--) {
        for (i = 0; i < block_size; i++) {
            SRAM[write_addr + Offset + i] = *pbuf;
            pbuf++;
        }

        Offset += block_size;
    }

    return 0;
}
