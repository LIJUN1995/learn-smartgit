#include "i2c.h"
#include "stdint.h"
#include "spi.h"
#include "sys/ioctl.h"
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test_cpld.h"
#include "common.h"

int i2c_write_data(unsigned short reg_address, unsigned char reg_data){

    cdfinger_spi_data *data = malloc(sizeof(cdfinger_spi_data));
    data->tx = malloc(sizeof(unsigned char)*6);
    data->rx = malloc(sizeof(unsigned char)*6);
    if(data == NULL || data->tx == NULL || data->rx == NULL){
        return -1;
    }

    memset(data->tx, 0x66, 6);
    memset(data->rx, 0x00, 6);

    data->tx[0]     = 0x82;
    data->length    = 4;
    data->tx_length = 4;

    do{
        spi_send_data(data);
        printf("1 i2c write data = 0x%02x!!!!!!\n",data->rx[3]);
    }while(data->rx[3]&0x80 == 0x80);

    /*set slaver device id*/
    
    data->tx[0]     = 0x81;
    data->tx[1]     = I2C_SLAVER_WRITE_DEVICE_ID;
    data->tx[2]     = 0x66;
    data->length    = 3;
    data->tx_length = 3;
    spi_send_data(data);
    /*set reg_address and reg_data*/
    data->tx[0]     = 0x85;
    data->tx[1]     = 0x00;
    data->tx[2]     = 0x66;
    data->tx[3]     = (uint8_t)(reg_address);
    data->tx[4]     = reg_data;
    data->length    = 5;
    data->tx_length = 5;
    spi_send_data(data);
    /*set the number of bytes to send*/
    data->tx[0]     = 0x83;
    data->tx[1]     = 0x02;
    data->tx[2]     = 0x66;
    data->length    = 3;
    data->tx_length = 3;
    spi_send_data(data);

    /*power on*/
    data->tx[0]     = 0x87;
    data->tx[1]     = 0x80;
    data->tx[2]     = 0x66;
    data->length    = 3;
    data->tx_length = 3;
    spi_send_data(data);
    data->tx[0]     = 0x87;
    data->tx[1]     = 0x00;
    data->tx[2]     = 0x66;
    data->length    = 3;
    data->tx_length = 3;
    spi_send_data(data);

    do{
        memset(data->tx, 0x66, 6);
        memset(data->rx, 0x00, 6);
        data->tx[0]     = 0x82;
        data->length    = 4;
        data->tx_length = 4;
        spi_send_data(data);
        printf("2 i2c write data = 0x%02x!!!!!!\n",data->rx[3]);
    }while(data->rx[3] != 0x08);

    free(data->tx);
    free(data->rx);
    free(data);
    
    return 0;
}

char i2c_read_data(uint16_t reg_address){

    cdfinger_spi_data *data = malloc(sizeof(cdfinger_spi_data));
    data->tx = malloc(sizeof(unsigned char)*6);
    data->rx = malloc(sizeof(unsigned char)*6);
    if(data == NULL || data->tx == NULL || data->rx == NULL){
        printf("malloc default=====>i2c_read_data\n");
        return -1;
    }
    memset(data->tx, 0x66, 6);
    memset(data->rx, 0x00, 6);

    do{
        memset(data->tx, 0x66, 6);
        data->tx[0]     = 0x82;
        data->length    = 4;
        data->tx_length = 4;
        ioctl(m_fd, CDFINGER_SPI_WRITE_AND_READ, data);
    }while(data->rx[3]&0x80 == 0x80);
    //printf("i2c is not busy!====1\n");
    //getchar();
    /*set slaver device id*/
    data->tx[0]     = 0x81;
    data->tx[1]     = I2C_SLAVER_READ_DEVICE_ID;
    data->tx[2]     = 0x66;//0x00
    data->length    = 3;
    data->tx_length = 3;
    ioctl(m_fd, CDFINGER_SPI_WRITE_AND_READ, data);
    //printf("slaver device id set done!\n");
    //getchar();
    /*set reg_address and reg_data*/
    data->tx[0]     = 0x85;
    data->tx[1]     = 0x00;
    data->tx[2]     = 0x66;
    data->tx[3]     = (uint8_t)(reg_address);
    data->length    = 4;
    data->tx_length = 4;
    ioctl(m_fd, CDFINGER_SPI_WRITE_AND_READ, data);
    //printf("reg_address set done!\n");
    //getchar();
    /*set the number of bytes to send*/
    data->tx[0]     = 0x83;
    data->tx[1]     = 0x00;
    data->tx[2]     = 0x66;
    data->length    = 3;
    data->tx_length = 3;
    ioctl(m_fd, CDFINGER_SPI_WRITE_AND_READ, data);
    //printf("the count to send set done!\n");
    //getchar();
    /*power on*/
    data->tx[0]     = 0x87;
    data->tx[1]     = 0x90;//1001 1000
    data->tx[2]     = 0x66;
    data->length    = 3;
    data->tx_length = 3;
    ioctl(m_fd, CDFINGER_SPI_WRITE_AND_READ, data);
    data->tx[0]     = 0x87;
    data->tx[1]     = 0x10;//
    data->tx[2]     = 0x66;
    ioctl(m_fd, CDFINGER_SPI_WRITE_AND_READ, data);
    //printf("power on!\n");
    //getchar();
    do{
        memset(data->tx, 0x66, 6);
        memset(data->tx, 0x00, 6);
        data->tx[0]     = 0x82;
        data->length    = 4;
        data->tx_length = 4;
        ioctl(m_fd, CDFINGER_SPI_WRITE_AND_READ, data);
    }while(data->rx[3] != 0x0c);
    //printf("i2c is not busy!===2\n");
    //getchar();
    data->tx[0]     = 0x86;
    data->tx[1]     = 0x00;
    data->tx[2]     = 0x66;
    data->tx[2]     = 0x66;
    data->length    = 4;
    data->tx_length = 4;
    //printf("!!!!!!!!!!1\n");
    ioctl(m_fd, CDFINGER_SPI_WRITE_AND_READ, data);
    //printf("get the data from i2c done!\n");
    //getchar();
    uint8_t value = data->rx[3];
    free(data->tx);
    free(data->rx);
    free(data);
    
    return value;
}