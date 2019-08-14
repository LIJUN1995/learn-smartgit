#include "spi.h"
#include <stdint.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include "sys/ioctl.h"
#include <stdio.h>
#include "test_cpld.h"
#include "common.h"

uint8_t *ptr = NULL;
int num = 0;

char spi_send_data(cdfinger_spi_data *data){
    uint8_t ret = ioctl(m_fd, CDFINGER_SPI_WRITE_AND_READ, data);
    if(ret < 0){
        printf("send data by spi default!\n");
        return ret;
    }
    return 0;
    
}

/****************************************************
 * @brief: start flash
 * 
 * @param: void
 * 
 * return: void
 * **************************************************/
void init_flash(void)
{
    char ret = 0;
    uint8_t tx[3] = {0};
    uint8_t rx[3] = {0};
    cdfinger_spi_data data;
    tx[0] = 0x00;
    tx[1] = 0x66;
    tx[2] = 0x66;
    data.tx = tx;
    data.rx = rx;
    data.tx_length = 3;
    data.length = 3;
    ret = spi_send_data(&data);
    if(ret != 0){
        printf("spi_send_data error!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    }
}

/****************************************************
 * @brief: if busy to judge the device 
 * 
 * @param: void
 * 
 * return: 0--no busy
 *         -1--busy
 * **************************************************/
int judgeFlashBusy(void)
{
    char ret = 0;
    uint8_t tx[3] = {0};
    uint8_t rx[3] = {0};
    cdfinger_spi_data data;
    tx[0] = 0x05;
    tx[1] = 0x66;
    tx[2] = 0x66;
    data.tx = tx;
    data.rx = rx;
    data.tx_length = 3;
    data.length = 3;
    spi_send_data(&data);
    if(rx[2]&0x01 == 0x01){
        printf("the device is busy!--------rx[2]=%02x\n",rx[2]);
        return -1;
    }else{
        return 0;
    }
}

/****************************************************
 * @brief: write enable for flash(set WEL bit 1) 
 * 
 * @param: void
 * 
 * return: void
 * **************************************************/
void writeEnable(void)
{
    uint8_t tx[1] = {0};
    uint8_t rx[1] = {0};
    cdfinger_spi_data data;
    tx[0] = 0x06;
    data.tx = tx;
    data.rx = rx;
    data.tx_length = 1;
    data.length = 1;
    spi_send_data(&data);
}

/****************************************************
 * @brief: earse the specified area inside the flash
 * 
 * @param: the Earse instruction for flash
 *         0x20--sector earse: 4K
 *         0xd8--block earse: 64K 
 *         0x52--half block earse: 32K
 *         0xc7/0x60--chip earse
 * @param: the base address of operation
 * 
 * return: void
 * **************************************************/
void erase_flash(uint8_t cmd,uint32_t addr)
{
    int ret = 0;
    uint8_t tx[4] = {0};
    uint8_t rx[4] = {0};
    cdfinger_spi_data data;
    data.tx = tx;
    data.rx = rx;

    do{
        ret = judgeFlashBusy();
        if(ret==-1)
            usleep(200*1000);
    }while(ret == -1);

    writeEnable();

    tx[0] = cmd;
    tx[1] = (addr>>16)&0x000000ff;
    tx[2] = (addr>>8)&0x000000ff;
    tx[3] = addr&0x000000ff;
    data.tx_length = 4;
    data.length = 4;
    spi_send_data(&data);

    do{
        ret = judgeFlashBusy();
        if(ret==-1){
            printf("the flash is busy!\n");
            usleep(200*1000);
        }
    }while(ret == -1);
}

/****************************************************
 * @brief: write the specified area inside the flash
 * 
 * @param: the Write instruction for flash
 *         0x02--page program: 256 bytes
 * @param: the base address of operation
 * @param: the data what you want to write
 * @param: the length of data(max:256 bytes)
 * 
 * return: void
 * **************************************************/
void page_program(uint8_t cmd,uint32_t addr,uint8_t *pp_data, int len)
{
    int ret = 0;
    uint8_t tx[260] = {0};
    uint8_t rx[260] = {0};
    cdfinger_spi_data data;
    data.tx = tx;
    data.rx = rx;

    do{
        ret = judgeFlashBusy();
        if(ret==-1)
            usleep(200*1000);
    }while(ret == -1);

    writeEnable();

    tx[0] = cmd;
    tx[1] = (addr>>16)&0x000000ff;
    tx[2] = (addr>>8)&0x000000ff;
    tx[3] = addr&0x000000ff;
    memcpy(tx+4,pp_data,len);
    data.tx_length = len+4;
    data.length = len+4;
    spi_send_data(&data);

    do{
        ret = judgeFlashBusy();
        if(ret==-1)
            usleep(200*1000);
    }while(ret == -1);
}

/****************************************************
 * @brief: write random data to the specified area inside the flash
 * 
 * @param: the base address of operation
 * @param: the data what you want to write
 * @param: the length of data
 * 
 * return: void
 * **************************************************/
void page_program_size(uint32_t addr,uint8_t *pp_data, int len)
{
    int i = 0;
    int e_count = 0;
    int p_count = 0;
    int remainder = 0;

    //if(len/(64*1024) != 0)
    if(len%(64*1024) != 0){
        e_count = len/(64*1024)+1;
    }else{
        e_count = len/(64*1024);
    }
    for(i=0;i<e_count;i++){
        printf("e_count==%d, i=%d\n",e_count,i);
        erase_flash(0x20,addr+i*(64*1024));
    }

    if(len <= 256){
        page_program(0x02,addr,pp_data,len);
    }else{
        p_count = len/256;
        remainder = len%256;
        for(i=0;i<p_count;i++){
            printf("p_count==%d,i=%d\n",p_count,i);
            page_program(0x02,addr+i*256,pp_data+i*256,256);
        }
        if(remainder != 0){
            page_program(0x02,addr+i*256,pp_data+i*256,remainder);
        }
    }
}

/****************************************************
 * @brief: read the specified area inside the flash
 * 
 * @param: the read instruction for flash
 *         0x03--read data
 * @param: the base address of operation
 * @param: read data to save in the buffer
 * @param: the length of data
 * 
 * return: void
 * **************************************************/
void read_flash(uint8_t cmd,uint32_t addr,uint8_t *rd_data, int len)
{
    int ret = 0;
    uint8_t *tx = (uint8_t *)malloc(len+4);
    uint8_t *rx = (uint8_t *)malloc(len+4);;
    cdfinger_spi_data data;
    data.tx = tx;
    data.rx = rx;

    do{
        ret = judgeFlashBusy();
        if(ret==-1)
            usleep(200*1000);
    }while(ret == -1);

    tx[0] = cmd;
    tx[1] = (addr>>16)&0x000000ff;
    tx[2] = (addr>>8)&0x000000ff;
    tx[3] = addr&0x000000ff;
    data.tx_length = len+4;
    data.length = len+4;
    spi_send_data(&data);
    memcpy(rd_data,rx+4,len);

    do{
        ret = judgeFlashBusy();
        if(ret==-1)
            usleep(200*1000);
    }while(ret == -1);
    
    free(tx);
    free(rx);
}

/****************************************************
 * @brief: check if operation is finished
 * 
 * @param: void
 * 
 * return: void
 * **************************************************/
void CheckBusy(void)
{
    uint8_t tx[5] = {0}; 
    uint8_t rx[5] = {0};
    cdfinger_spi_data data;
    data.tx = tx;
    data.rx = rx;

    tx[0] = 0xf0;
    data.length = 5;
    data.tx_length = 5;

    do{
        ioctl(m_fd,CDFINGER_CONTROL_RESET,0);
        spi_send_data(&data);
        ioctl(m_fd,CDFINGER_CONTROL_RESET,1);
        if((rx[4]&0x80) == 0x80){
            printf("the Device is busy: status=%02x\n",rx[4]);
            usleep(100*1000);
        }
    }while((rx[4]&0x80) == 0x80);
    printf("the Device ID is idle: status = %02x\n",rx[4]);
}

/****************************************************
 * @brief: check ID code
 * 
 * @param: void
 * 
 * return: void
 * **************************************************/
void CheckIdCode(void)
{
    uint8_t tx[8] = {0}; 
    uint8_t rx[8] = {0};
    cdfinger_spi_data data;
    data.tx = tx;
    data.rx = rx;

rety:
    tx[0] = 0xe0;
    data.length = 8;
    data.tx_length = 8;

    ioctl(m_fd,CDFINGER_CONTROL_RESET,0);
    spi_send_data(&data);
    ioctl(m_fd,CDFINGER_CONTROL_RESET,1);

    while(rx[4]!=0x61 || rx[5]!=0x2b || rx[6]!=0x20 || rx[7]!=0x43){
        int i = 0;
        printf("the Device ID error:");
        for(i=0;i<8;i++)
            printf("rx[%d]=%02x   ",i,rx[i]);
        printf("\n");
        goto rety;
    }
}

/****************************************************
 * @brief: enable background program
 * 
 * @param: void
 * 
 * return: void
 * **************************************************/
void EnableBP(void)
{
    uint8_t tx[4] = {0}; 
    uint8_t rx[4] = {0};
    cdfinger_spi_data data;
    data.tx = tx;
    data.rx = rx;

    tx[0] = 0x74;
    tx[1] = 0x08;
    data.length = 4;
    data.tx_length = 4;
    ioctl(m_fd,CDFINGER_CONTROL_RESET,0);
    spi_send_data(&data);
    ioctl(m_fd,CDFINGER_CONTROL_RESET,1);
    
    int i = 0;
    printf("%s:%d debug:",__FUNCTION__,__LINE__);
    for(i=0;i<4;i++)
        printf("rx[%d]=%02x   ",i,rx[i]);
    printf("\n");
    CheckBusy();
}

/****************************************************
 * @brief: erase config flash inside chip
 * 
 * @param: void
 * 
 * return: void
 * **************************************************/
void EraseInternalFlash(void)
{
    uint8_t tx[4] = {0}; 
    uint8_t rx[4] = {0};
    cdfinger_spi_data data;
    data.tx = tx;
    data.rx = rx;

    tx[0] = 0x0e;
    tx[1] = 0x04;
    data.length = 4;
    data.tx_length = 4;
    ioctl(m_fd,CDFINGER_CONTROL_RESET,0);
    spi_send_data(&data);
    ioctl(m_fd,CDFINGER_CONTROL_RESET,1);

    int i = 0;
    printf("%s:%d debug:",__FUNCTION__,__LINE__);
    for(i=0;i<4;i++)
        printf("rx[%d]=%02x   ",i,rx[i]);
    printf("\n");

    CheckBusy();
}

/****************************************************
 * @brief: reset config flash address
 * 
 * @param: void
 * 
 * return: void
 * **************************************************/
void ResetFlashAddress(void)
{
    uint8_t tx[4] = {0}; 
    uint8_t rx[4] = {0};
    cdfinger_spi_data data;
    data.tx = tx;
    data.rx = rx;

    tx[0] = 0x46;
    data.length = 4;
    data.tx_length = 4;
    ioctl(m_fd,CDFINGER_CONTROL_RESET,0);
    spi_send_data(&data);
    ioctl(m_fd,CDFINGER_CONTROL_RESET,1);
    CheckBusy();
}

void ProgramUserCode(void)
{
    uint8_t tx[8] = {0}; 
    uint8_t rx[8] = {0};
    cdfinger_spi_data data;
    data.tx = tx;
    data.rx = rx;

    tx[0] = 0xc2;
    tx[4] = 0x00;
    tx[5] = 0x00;
    tx[6] = 0x00;
    tx[7] = 0x00;
    data.length = 8;
    data.tx_length = 8;
    ioctl(m_fd,CDFINGER_CONTROL_RESET,0);
    spi_send_data(&data);
    ioctl(m_fd,CDFINGER_CONTROL_RESET,1);
    CheckBusy();
}

void WriteFeatureRow(void)
{
    uint8_t tx[64] = {0}; 
    uint8_t rx[64] = {0};
    cdfinger_spi_data data;
    data.tx = tx;
    data.rx = rx;

    tx[0] = 0x46;
    tx[1] = 0x02;
    data.length = 4;
    data.tx_length = 4;
    ioctl(m_fd,CDFINGER_CONTROL_RESET,0);
    spi_send_data(&data);
    ioctl(m_fd,CDFINGER_CONTROL_RESET,1);
    CheckBusy();

    tx[0] = 0xe4;
    tx[1] = 0x00;
    data.length = 12;
    data.tx_length = 12;
    ioctl(m_fd,CDFINGER_CONTROL_RESET,0);
    spi_send_data(&data);
    ioctl(m_fd,CDFINGER_CONTROL_RESET,1);
    CheckBusy();
    usleep(10*1000);

    tx[0] = 0xe7;
    tx[1] = 0x00;
    data.length = 12;
    data.tx_length = 12;
    ioctl(m_fd,CDFINGER_CONTROL_RESET,0);
    spi_send_data(&data);
    ioctl(m_fd,CDFINGER_CONTROL_RESET,1);
    CheckBusy();
    usleep(10*1000);

    tx[0] = 0xf8;
    tx[4] = 0x05;
    tx[5] = 0x20;
    data.length = 6;
    data.tx_length = 6;
    ioctl(m_fd,CDFINGER_CONTROL_RESET,0);
    spi_send_data(&data);
    ioctl(m_fd,CDFINGER_CONTROL_RESET,1);
    CheckBusy();
    usleep(10*1000);

    tx[0] = 0xfb;
    tx[4] = 0x00;
    tx[5] = 0x00;
    data.length = 6;
    data.tx_length = 6;
    ioctl(m_fd,CDFINGER_CONTROL_RESET,0);
    spi_send_data(&data);
    ioctl(m_fd,CDFINGER_CONTROL_RESET,1);
    CheckBusy();
    usleep(10*1000);
}

void SPI_Verify(void)
{
    uint8_t tx[256] = {0}; 
    uint8_t rx[256] = {0};
    cdfinger_spi_data data;
    data.tx = tx;
    data.rx = rx;

    tx[0] = 0x73;
    tx[1] = 0x10;
    tx[2] = 0xff;
    tx[3] = 0xff;
    data.length = 4;
    data.tx_length = 4;
    ioctl(m_fd,CDFINGER_CONTROL_RESET,0);
    spi_send_data(&data);
    ioctl(m_fd,CDFINGER_CONTROL_RESET,1);
    CheckBusy();

    memset(tx,0,256);
    data.length = 256;
    data.tx_length = 256;
    ioctl(m_fd,CDFINGER_CONTROL_RESET,0);
    spi_send_data(&data);
    ioctl(m_fd,CDFINGER_CONTROL_RESET,1);

    int i = 0;
    printf("%s:%d debug:",__FUNCTION__,__LINE__);
    for(i=0;i<256;i++)
        printf("rx[%d]=%02x   ptr[%d]=%02x\n",i,rx[i],i,ptr[i]);
    printf("\n");
}

void ReadUserCode(void)
{
    uint8_t tx[8] = {0}; 
    uint8_t rx[8] = {0};
    cdfinger_spi_data data;
    data.tx = tx;
    data.rx = rx;

    tx[0] = 0xc0;
    data.length = 8;
    data.tx_length = 8;
    ioctl(m_fd,CDFINGER_CONTROL_RESET,0);
    spi_send_data(&data);
    ioctl(m_fd,CDFINGER_CONTROL_RESET,1);

    int i = 0;
    printf("%s:%d debug:",__FUNCTION__,__LINE__);
    for(i=4;i<8;i++)
        printf("rx[%d]=%02x   ",i,rx[i]);
    printf("\n");

    CheckBusy();
}

void ProgramDoneBit(void)
{
    uint8_t tx[4] = {0}; 
    uint8_t rx[4] = {0};
    cdfinger_spi_data data;
    data.tx = tx;
    data.rx = rx;

    tx[0] = 0x5e;
    data.length = 4;
    data.tx_length = 4;
    ioctl(m_fd,CDFINGER_CONTROL_RESET,0);
    spi_send_data(&data);
    ioctl(m_fd,CDFINGER_CONTROL_RESET,1);
    CheckBusy();
}

void ISC_Disable(void)
{
    uint8_t tx[4] = {0}; 
    uint8_t rx[4] = {0};
    cdfinger_spi_data data;
    data.tx = tx;
    data.rx = rx;

    tx[0] = 0x26;
    tx[1] = 0xff;
    tx[2] = 0xff;
    tx[3] = 0xff;
    data.length = 4;
    data.tx_length = 4;
    ioctl(m_fd,CDFINGER_CONTROL_RESET,0);
    spi_send_data(&data);
    ioctl(m_fd,CDFINGER_CONTROL_RESET,1);
    CheckBusy();

    // tx[0] = 0x00;
    // data.length = 2;
    // data.tx_length = 2;
    // ioctl(m_fd,CDFINGER_CONTROL_RESET,0);
    // spi_send_data(&data);
    // ioctl(m_fd,CDFINGER_CONTROL_RESET,1);
    // CheckBusy();
}

void Bypass(void)
{
    uint8_t tx[4] = {0}; 
    uint8_t rx[4] = {0};
    cdfinger_spi_data data;
    data.tx = tx;
    data.rx = rx;

    tx[0] = 0xff;
    tx[1] = 0xff;
    tx[2] = 0xff;
    tx[3] = 0xff;
    data.length = 4;
    data.tx_length = 4;
    ioctl(m_fd,CDFINGER_CONTROL_RESET,0);
    spi_send_data(&data);
    ioctl(m_fd,CDFINGER_CONTROL_RESET,1);
    CheckBusy();
}

void Resfresh(void)
{
    uint8_t tx[4] = {0}; 
    uint8_t rx[4] = {0};
    cdfinger_spi_data data;
    data.tx = tx;
    data.rx = rx;

    tx[0] = 0x79;
    tx[1] = 0xff;
    tx[2] = 0xff;
    tx[3] = 0xff;
    data.length = 4;
    data.tx_length = 4;
    ioctl(m_fd,CDFINGER_CONTROL_RESET,0);
    spi_send_data(&data);
    ioctl(m_fd,CDFINGER_CONTROL_RESET,1);
    CheckBusy();
}

/****************************************************
 * @brief: write into flash by one row
 * 
 * @param: buffer of one row code
 * 
 * return: void
 * **************************************************/
void SPI_Program_Row(uint8_t *ptr,int len)
{
    uint8_t tx[20] = {0}; 
    uint8_t rx[20] = {0};
    cdfinger_spi_data data;
    data.tx = tx;
    data.rx = rx;

    memcpy(tx+4,ptr,len);

    tx[0] = 0x70;
    data.length = 20;
    data.tx_length = 20;
    ioctl(m_fd,CDFINGER_CONTROL_RESET,0);
    spi_send_data(&data);
    ioctl(m_fd,CDFINGER_CONTROL_RESET,1);
    CheckBusy();
}

/****************************************************
 * @brief: write into flash for raw code
 * 
 * @param: buffer of raw code
 * @param: length of raw code
 * 
 * return: void
 * **************************************************/
void WriteCodeInternalFlash(uint8_t *p_code,int len)
{
    int i = 0;
    int count = len/16;
    int reminder = len%16;

    printf("%s:%d: count=%d, reminder=%d\n",__FUNCTION__,__LINE__,count,reminder);
    for(i=0;i<count;i++){
        SPI_Program_Row(p_code+i*16,16);
    }
    if(reminder != 0)
        SPI_Program_Row(p_code+i*16,reminder);
}

/****************************************************
 * @brief: program code CFG flash, don't prgram UFM
 * 
 * @param: void
 * 
 * return: void
 * **************************************************/
int ProgramInternalFlash(void)
{
    FILE *fp=fopen("/data/demo.bin","rb+");
	int ret = 0;
	if(fp == NULL){
		printf("open file error\n");
		exit(1);
	}

	fseek(fp,0L,SEEK_END);
	num = ftell(fp);
    printf("num = %d\n",num);
    fseek(fp,0L,SEEK_SET);
    ptr=(uint8_t *)malloc(sizeof(uint8_t)*num);
    ret = fread(ptr,sizeof(uint8_t),num,fp);
    if(ret != num){
        printf("fread error!!!!!!!!!!!!!!!!!!!!!!!\n");
        exit(1);
    }

    WriteCodeInternalFlash(ptr,num);

    //free(ptr);
	fclose(fp);
    return 0;
}