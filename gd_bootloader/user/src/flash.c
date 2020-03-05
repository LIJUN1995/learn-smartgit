/********************************************************************************
*文 件 名：flash
*文件功能：配置flash可读写
*版    本：v0.1
*时    间：2018.11.15
********************************************************************************/
#include "main.h"

//int32_t data_32_1[] = {-1,-2,-3,-4,-5,-6,-7,-8,-9,-10};
//int32_t data_32_2[10];
//int16_t data_16_1[] = {1,2,3,4,5,6,7,8,9,10};
//int16_t data_16_2[10];
//int8_t data_8_1[] = {1,2,3,4,5,6,7,8,9,10};
//int8_t data_8_2[10];
/*!
    \brief      gets the sector of a given address
    \param[in]  address: a given address(0x08000000~0x080FFFFF)
    \param[out] none
    \retval     the sector of a given address
*/
uint32_t fmc_sector_get(uint32_t address)
{
    uint32_t sector = 0;
    if((address < ADDR_FMC_SECTOR_1) && (address >= ADDR_FMC_SECTOR_0)){
        sector = CTL_SECTOR_NUMBER_0;
    }else if((address < ADDR_FMC_SECTOR_2) && (address >= ADDR_FMC_SECTOR_1)){
        sector = CTL_SECTOR_NUMBER_1;
    }else if((address < ADDR_FMC_SECTOR_3) && (address >= ADDR_FMC_SECTOR_2)){
        sector = CTL_SECTOR_NUMBER_2;
    }else if((address < ADDR_FMC_SECTOR_4) && (address >= ADDR_FMC_SECTOR_3)){
        sector = CTL_SECTOR_NUMBER_3;  
    }else if((address < ADDR_FMC_SECTOR_5) && (address >= ADDR_FMC_SECTOR_4)){
        sector = CTL_SECTOR_NUMBER_4;
    }else if((address < ADDR_FMC_SECTOR_6) && (address >= ADDR_FMC_SECTOR_5)){
        sector = CTL_SECTOR_NUMBER_5;
    }else if((address < ADDR_FMC_SECTOR_7) && (address >= ADDR_FMC_SECTOR_6)){
        sector = CTL_SECTOR_NUMBER_6;
    }else if((address < ADDR_FMC_SECTOR_8) && (address >= ADDR_FMC_SECTOR_7)){
        sector = CTL_SECTOR_NUMBER_7;  
    }else if((address < ADDR_FMC_SECTOR_9) && (address >= ADDR_FMC_SECTOR_8)){
        sector = CTL_SECTOR_NUMBER_8;
    }else if((address < ADDR_FMC_SECTOR_10) && (address >= ADDR_FMC_SECTOR_9)){
        sector = CTL_SECTOR_NUMBER_9;
    }else if((address < ADDR_FMC_SECTOR_11) && (address >= ADDR_FMC_SECTOR_10)){
        sector = CTL_SECTOR_NUMBER_10;
    }else{
        sector = CTL_SECTOR_NUMBER_11;
    }
    return sector;
}

/*!
    \brief      erases the sector of a given sector number
    \param[in]  fmc_sector: a given sector number
      \arg        CTL_SECTOR_NUMBER_0: sector 0
      \arg        CTL_SECTOR_NUMBER_1: sector 1
      \arg        CTL_SECTOR_NUMBER_2: sector 2
      \arg        CTL_SECTOR_NUMBER_3��sector 3
      \arg        CTL_SECTOR_NUMBER_4: sector 4
      \arg        CTL_SECTOR_NUMBER_5: sector 5
      \arg        CTL_SECTOR_NUMBER_6: sector 6
      \arg        CTL_SECTOR_NUMBER_7��sector 7
      \arg        CTL_SECTOR_NUMBER_8: sector 8
      \arg        CTL_SECTOR_NUMBER_9: sector 9
      \arg        CTL_SECTOR_NUMBER_10: sector 10
      \arg        CTL_SECTOR_NUMBER_11��sector 11
    \param[out] none
    \retval     none
*/
void fmc_erase_sector(uint32_t fmc_sector)
{
    /* unlock the flash program erase controller */
    fmc_unlock(); 
    /* clear pending flags */
    fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_OPERR | FMC_FLAG_WPERR | FMC_FLAG_PGMERR | FMC_FLAG_PGSERR);
    /* wait the erase operation complete*/
    if(FMC_READY != fmc_sector_erase(fmc_sector)){
        while(1);
    }
    /* lock the flash program erase controller */
    fmc_lock();
}

/*!
    \brief      write 32 bit length data to a given address
    \param[in]  address: a given address(0x08000000~0x080FFFFF)
    \param[in]  length: data length
    \param[in]  data_32: data pointer
    \param[out] none
    \retval     none
*/
void fmc_write_32bit_data(uint32_t address, uint16_t length, int32_t* data_32)
{
    uint16_t StartSector, EndSector,i;
    /* unlock the flash program erase controller */
    fmc_unlock();
    /* clear pending flags */
    fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_OPERR | FMC_FLAG_WPERR | FMC_FLAG_PGMERR | FMC_FLAG_PGSERR);
    /* get the number of the start and end sectors */
    StartSector = fmc_sector_get(address);
    EndSector = fmc_sector_get(address + 4*length);
    /* each time the sector number increased by 8, refer to the sector definition */
    for(i = StartSector; i < EndSector; i += 8){
        if(FMC_READY != fmc_sector_erase(i)){
            while(1);
        }
    }
    /* write data_32 to the corresponding address */
    for(i=0; i<length; i++){
        if(FMC_READY == fmc_word_program(address, data_32[i])){
            address = address + 4;
        }else{ 
            while(1);
        }
    }
    /* lock the flash program erase controller */
    fmc_lock();
}

/*!
    \brief      read 32 bit length data from a given address
    \param[in]  address: a given address(0x08000000~0x080FFFFF)
    \param[in]  length: data length
    \param[in]  data_32: data pointer
    \param[out] none
    \retval     none
*/
void fmc_read_32bit_data(uint32_t address, uint16_t length, int32_t* data_32)
{
    uint8_t i;
    for(i=0; i<length; i++){
        data_32[i] = *(__IO int32_t*)address;
        address=address + 4;
    }
}

/*!
    \brief      write 16 bit length data to a given address
    \param[in]  address: a given address(0x08000000~0x080FFFFF)
    \param[in]  length: data length
    \param[in]  data_16: data pointer
    \param[out] none
    \retval     none
*/
void fmc_write_16bit_data(uint32_t address, uint16_t length, int16_t* data_16)
{
    uint16_t StartSector, EndSector,i;
    /* unlock the flash program erase controller */
    fmc_unlock();
    /* clear pending flags */
    fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_OPERR | FMC_FLAG_WPERR | FMC_FLAG_PGMERR | FMC_FLAG_PGSERR);
    /* get the number of the start and end sectors */
    StartSector = fmc_sector_get(address);
    EndSector = fmc_sector_get(address + 2*length);
    /* each time the sector number increased by 8, refer to the sector definition */
    for(i = StartSector; i < EndSector; i += 8){
        if(FMC_READY != fmc_sector_erase(i)){
            while(1);
        }
    }
    /* write data_16 to the corresponding address */
    for(i=0; i<length; i++){
        if(FMC_READY == fmc_halfword_program(address, data_16[i])){
            address = address + 2;
        }else{ 
            while(1);
        }
    }
    /* lock the flash program erase controller */
    fmc_lock();
}

/*!
    \brief      read 16 bit length data to a given address
    \param[in]  address: a given address(0x08000000~0x080FFFFF)
    \param[in]  length: data length
    \param[in]  data_16: data pointer
    \param[out] none
    \retval     none
*/
void fmc_read_16bit_data(uint32_t address, uint16_t length, int16_t* data_16)
{
    uint8_t i;
    for(i=0; i<length; i++){
        data_16[i] = *(__IO int16_t*)address;
        address = address + 2;
    }
}

/*!
    \brief      write 8 bit length data to a given address
    \param[in]  address: a given address(0x08000000~0x080FFFFF)
    \param[in]  length: data length
    \param[in]  data_8: data pointer
    \param[out] none
    \retval     none
*/
void fmc_write_8bit_data(uint32_t address, uint16_t length, int8_t* data_8)
{
    uint16_t StartSector, EndSector,i;
    /* unlock the flash program erase controller */
    fmc_unlock();
    /* clear pending flags */
    fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_OPERR | FMC_FLAG_WPERR | FMC_FLAG_PGMERR | FMC_FLAG_PGSERR);
    /* get the number of the start and end sectors */
    StartSector = fmc_sector_get(address);
    EndSector = fmc_sector_get(address + length);
    /* each time the sector number increased by 8, refer to the sector definition */
    for(i = StartSector; i < EndSector; i += 8){
        if(FMC_READY != fmc_sector_erase(i)){
            while(1);
        }
    }
    /* write data_8 to the corresponding address */
    for(i=0; i<length; i++){
        if(FMC_READY == fmc_byte_program(address, data_8[i])){
            address++;
        }else{ 
            while(1);
        }
    }
    /* lock the flash program erase controller */
    fmc_lock();
}

/*!
    \brief      read 8 bit length data to a given address
    \param[in]  address: a given address(0x08000000~0x080FFFFF)
    \param[in]  length: data length
    \param[in]  data_8: data pointer
    \param[out] none
    \retval     none
*/
void fmc_read_8bit_data(uint32_t address, uint16_t length, int8_t* data_8)
{
    uint8_t i;
    for(i=0; i<length; i++){
        data_8[i] = *(__IO int8_t*)address;
        address++;
    }
}

#if 0
/********************************************************************************
*函 数 名：flash_init
*函数功能：将配置写进flash
*输    入：
*输    出：
*备    注：
********************************************************************************/
void FingerTech_FlashConfig(void)
{
    chip_cfg temp_cfg={0};
    uint16_t camera_cfg_for_flash[9];    
    fmc_read_16bit_data(FLASH_USER_START_ADDR, 9, camera_cfg_for_flash);
    if(camera_cfg.update == 1 || camera_cfg_for_flash[0] != 0x55)
    {
#ifdef SPI_TRANSFER
        printf("FingerTech_FlashConfig~~~~~\n");
#endif
        fmc_erase_sector(CTL_SECTOR_NUMBER_11);
        ob_write_protection0_disable(OB_WP_11);
        camera_cfg_for_flash[1] = camera_cfg.exp_time;
        camera_cfg_for_flash[2] = camera_cfg.x;
        camera_cfg_for_flash[3] = camera_cfg.y;
        camera_cfg_for_flash[4] = camera_cfg.width;
        camera_cfg_for_flash[5] = camera_cfg.hight;
        camera_cfg_for_flash[6] = camera_cfg.full_size_w;
        camera_cfg_for_flash[7] = camera_cfg.update;
        camera_cfg_for_flash[8] = camera_cfg.update_program;
        fmc_write_16bit_data(FLASH_USER_START_ADDR, 9, camera_cfg_for_flash);
        fmc_read_16bit_data(FLASH_USER_START_ADDR, 9, camera_cfg_for_flash);
    }
    //else
    //{
    //    camera_cfg = temp_cfg;	
    //    printf("the flash is already be writen\n");	
    //}
#ifdef SPI_TRANSFER
    printf("exp_time = %d\n",camera_cfg_for_flash[1]);
    printf("x = %d\n",camera_cfg_for_flash[2]);
    printf("y = %d\n",camera_cfg_for_flash[3]);
    printf("width = %d\n",camera_cfg_for_flash[4]);
    printf("hight = %d\n",camera_cfg_for_flash[5]);
    printf("full_size_w = %d\n",camera_cfg_for_flash[6]);
    ("update = %d\n",camera_cfg_for_flash[7]);	
    printf("update_program = %d\n",camera_cfg_for_flash[8]);
#endif
}

/********************************************************************************
*函 数 名：update_camera_cfg
*函数功能：将配置写进flash
*输    入：
*输    出：
*备    注：
********************************************************************************/
void update_camera_cfg(void){
    chip_cfg temp_cfg={0};
    uint16_t camera_cfg_for_flash[9];   
#ifdef SPI_TRANSFER
    printf("update camera cfg~~~~~\n");
#endif
    fmc_erase_sector(CTL_SECTOR_NUMBER_11);
    ob_write_protection0_disable(OB_WP_11);
    camera_cfg_for_flash[0] = camera_cfg.stat;
    camera_cfg_for_flash[1] = camera_cfg.exp_time;
    camera_cfg_for_flash[2] = camera_cfg.x;
    camera_cfg_for_flash[3] = camera_cfg.y;
    camera_cfg_for_flash[4] = camera_cfg.width;
    camera_cfg_for_flash[5] = camera_cfg.hight;
    camera_cfg_for_flash[6] = camera_cfg.full_size_w;
    camera_cfg_for_flash[7] = camera_cfg.update;
    camera_cfg_for_flash[8] = camera_cfg.update_program;
    fmc_write_16bit_data(FLASH_USER_START_ADDR, 9, camera_cfg_for_flash);
    fmc_read_16bit_data(FLASH_USER_START_ADDR, 9, camera_cfg_for_flash);
#ifdef SPI_TRANSFER
    printf("exp_time = %d\n",camera_cfg_for_flash[1]);
    printf("x = %d\n",camera_cfg_for_flash[2]);
    printf("y = %d\n",camera_cfg_for_flash[3]);
    printf("width = %d\n",camera_cfg_for_flash[4]);
    printf("hight = %d\n",camera_cfg_for_flash[5]);
    printf("full_size_w = %d\n",camera_cfg_for_flash[6]);
    printf("update = %d\n",camera_cfg_for_flash[7]);	
    printf("update_program = %d\n",camera_cfg_for_flash[8]);
#endif
}
#endif
