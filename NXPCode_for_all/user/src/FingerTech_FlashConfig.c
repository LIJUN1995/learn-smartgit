#include "main.h"

#define FLASH_FLEXSPI FLEXSPI
#define FLASH_SIZE 0x1000 /* 64Mb/KByte */

#define FLASH_SECTOR 200
#define SECTOR_SIZE 0x1000 /* 4K */

#define FLASH_PAGE_SIZE 256

#define NOR_CMD_LUT_SEQ_IDX_READID1             10
#define NOR_CMD_LUT_SEQ_IDX_READSTATUS1         11
#define NOR_CMD_LUT_SEQ_IDX_WRITEENABLE1        12
#define NOR_CMD_LUT_SEQ_IDX_ERASESECTOR1        13
#define NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_SINGLE1 14
#define NOR_CMD_LUT_SEQ_IDX_READ_NORMAL1				15

#define FLASH_BUSY_STATUS_POL 1
#define FLASH_BUSY_STATUS_OFFSET 0
#define FLASH_ERROR_STATUS_MASK 0x0e

extern chip_cfg camera_cfg;
//static uint8_t s_nor_program_buffer[256];
//static uint8_t s_nor_read_buffer[256];

/*******************************************************************************
 * Code
 ******************************************************************************/
flexspi_device_config_t deviceconfig = {
    .flexspiRootClk = 120000000,
    .flashSize = FLASH_SIZE,
    .CSIntervalUnit = kFLEXSPI_CsIntervalUnit1SckCycle,
    .CSInterval = 2,
    .CSHoldTime = 3,
    .CSSetupTime = 3,
    .dataValidTime = 0,
    .columnspace = 0,
    .enableWordAddress = 0,
    .AWRSeqIndex = 0,
    .AWRSeqNumber = 0,
    .ARDSeqIndex = 0,//Normal Read
    .ARDSeqNumber = 1,
    .AHBWriteWaitUnit = kFLEXSPI_AhbWriteWaitUnit2AhbCycle,
    .AHBWriteWaitInterval = 0,
};

/* Fixed 64 LUTs, suggest to reserve the LUTs in the front */
const uint32_t customLUT[64] = {
        /* Read ID */
        [4 * NOR_CMD_LUT_SEQ_IDX_READID1] =
            FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0xAB, kFLEXSPI_Command_DUMMY_SDR, kFLEXSPI_1PAD, 0x18),
        [4 * NOR_CMD_LUT_SEQ_IDX_READID1 + 1] =
            FLEXSPI_LUT_SEQ(kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, 0x04, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),
        /* Read extend parameters */
        [4 * NOR_CMD_LUT_SEQ_IDX_READSTATUS1] =
            FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x05, kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, 0x04),
        /* Write Enable */
        [4 * NOR_CMD_LUT_SEQ_IDX_WRITEENABLE1] =
            FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x06, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),
        /* Erase Sector  */
        [4 * NOR_CMD_LUT_SEQ_IDX_ERASESECTOR1] =
            FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x20, kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD, 0x18),
        /* Page Program - single mode */
        [4 * NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_SINGLE1] =
            FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x02, kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD, 0x18),
        [4 * NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_SINGLE1 + 1] =
            FLEXSPI_LUT_SEQ(kFLEXSPI_Command_WRITE_SDR, kFLEXSPI_1PAD, 0x04, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),
		    /* Normal read mode -SDR */
        [4 * NOR_CMD_LUT_SEQ_IDX_READ_NORMAL1] =
            FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x03, kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD, 0x18),
        [4 * NOR_CMD_LUT_SEQ_IDX_READ_NORMAL1 + 1] =
            FLEXSPI_LUT_SEQ(kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, 0x04, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),
};

status_t __attribute__((section("NonCacheable"))) flexspi_nor_write_enable(FLEXSPI_Type *base, uint32_t baseAddr)
{
    flexspi_transfer_t flashXfer;
    status_t status;

    /* Write neable */
    flashXfer.deviceAddress = baseAddr;
    flashXfer.port = kFLEXSPI_PortA1;
    flashXfer.cmdType = kFLEXSPI_Command;
    flashXfer.SeqNumber = 1;
    flashXfer.seqIndex = NOR_CMD_LUT_SEQ_IDX_WRITEENABLE1;

    status = FLEXSPI_TransferBlocking(base, &flashXfer);

    return status;
}

status_t __attribute__((section("NonCacheable"))) flexspi_nor_wait_bus_busy(FLEXSPI_Type *base)
{
    /* Wait status ready. */
    bool isBusy;
    uint32_t readValue;
    status_t status;
    flexspi_transfer_t flashXfer;

    flashXfer.deviceAddress = 0;
    flashXfer.port = kFLEXSPI_PortA1;
    flashXfer.cmdType = kFLEXSPI_Read;
    flashXfer.SeqNumber = 1;
    flashXfer.seqIndex = NOR_CMD_LUT_SEQ_IDX_READSTATUS1;
    flashXfer.data = &readValue;
    flashXfer.dataSize = 4;

    do
    {
        status = FLEXSPI_TransferBlocking(base, &flashXfer);
        if (status != kStatus_Success)
        {
            return status;
        }
        if (FLASH_BUSY_STATUS_POL)
        {
            if (readValue & (1U << FLASH_BUSY_STATUS_OFFSET))
            {
                isBusy = true;
            }
            else
            {
                isBusy = false;
            }
        }
        else
        {
            if (readValue & (1U << FLASH_BUSY_STATUS_OFFSET))
            {
                isBusy = false;
            }
            else
            {
                isBusy = true;
            }
        }

    } while (isBusy);
	
		
    return status;
}

status_t __attribute__((section("NonCacheable"))) flexspi_nor_get_vendor_id(FLEXSPI_Type *base, uint8_t *vendorId)
{
    uint32_t temp;
    flexspi_transfer_t flashXfer;
    flashXfer.deviceAddress = 0;
    flashXfer.port = kFLEXSPI_PortA1;
    flashXfer.cmdType = kFLEXSPI_Read;
    flashXfer.SeqNumber = 1;
    flashXfer.seqIndex = NOR_CMD_LUT_SEQ_IDX_READID1;
    flashXfer.data = &temp;
    flashXfer.dataSize = 1;

    status_t status = FLEXSPI_TransferBlocking(base, &flashXfer);

    *vendorId = temp;

    return status;
}

status_t __attribute__((section("NonCacheable"))) flexspi_nor_flash_erase_sector(FLEXSPI_Type *base, uint32_t address)
{
    status_t status;
    flexspi_transfer_t flashXfer;

    /* Write enable */
    flashXfer.deviceAddress = address;
    flashXfer.port = kFLEXSPI_PortA1;
    flashXfer.cmdType = kFLEXSPI_Command;
    flashXfer.SeqNumber = 1;
    flashXfer.seqIndex = NOR_CMD_LUT_SEQ_IDX_WRITEENABLE1;

    status = FLEXSPI_TransferBlocking(base, &flashXfer);

    if (status != kStatus_Success)
    {
        return status;
    }

    flashXfer.deviceAddress = address;
    flashXfer.port = kFLEXSPI_PortA1;
    flashXfer.cmdType = kFLEXSPI_Command;
    flashXfer.SeqNumber = 1;
    flashXfer.seqIndex = NOR_CMD_LUT_SEQ_IDX_ERASESECTOR1;
    status = FLEXSPI_TransferBlocking(base, &flashXfer);

    if (status != kStatus_Success)
    {
        return status;
    }

    status = flexspi_nor_wait_bus_busy(base);

    return status;
}

status_t __attribute__((section("NonCacheable"))) flexspi_nor_flash_page_program(FLEXSPI_Type *base, uint32_t dstAddr, const uint32_t *src,size_t len)
{
    status_t status;
    flexspi_transfer_t flashXfer;

    /* Write neable */
    status = flexspi_nor_write_enable(base, dstAddr);

    if (status != kStatus_Success)
    {
        return status;
    }

    /* Prepare page program command */
    flashXfer.deviceAddress = dstAddr;
    flashXfer.port = kFLEXSPI_PortA1;
    flashXfer.cmdType = kFLEXSPI_Write;
    flashXfer.SeqNumber = 1;
    flashXfer.seqIndex = NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_SINGLE1;
    flashXfer.data = (uint32_t *)src;
    flashXfer.dataSize = len;
    status = FLEXSPI_TransferBlocking(base, &flashXfer);

    if (status != kStatus_Success)
    {
        return status;
    }

    status = flexspi_nor_wait_bus_busy(base);

    return status;
}

status_t __attribute__((section("NonCacheable"))) flexspi_nor_flash_read_sector(FLEXSPI_Type *base, uint32_t address,const uint32_t *src,size_t len)
{
	//uint32_t temp;
    flexspi_transfer_t flashXfer;
    flashXfer.deviceAddress = address;
    flashXfer.port = kFLEXSPI_PortA1;
    flashXfer.cmdType = kFLEXSPI_Read;
    flashXfer.SeqNumber = 1;
    flashXfer.seqIndex = NOR_CMD_LUT_SEQ_IDX_READ_NORMAL1;
    flashXfer.data = (uint32_t *)src;
    flashXfer.dataSize = len;
    status_t status = FLEXSPI_TransferBlocking(base, &flashXfer);
   // *vendorId = temp;
    return status;
}

void FingerTech_FlashConfig(void)
{
	uint32_t status = 0;
	uint8_t vendorID = 0;
	uint32_t intmask = 0;
	chip_cfg temp_cfg={0};
	
	/* Update LUT table. */
  FLEXSPI_UpdateLUT(FLASH_FLEXSPI,NOR_CMD_LUT_SEQ_IDX_READID1*4,&customLUT[NOR_CMD_LUT_SEQ_IDX_READID1*4], 64);
	
	intmask=DisableGlobalIRQ();
	status = flexspi_nor_get_vendor_id(FLASH_FLEXSPI, &vendorID);
	EnableGlobalIRQ(intmask);
	
	intmask=DisableGlobalIRQ();
	flexspi_nor_flash_read_sector(FLASH_FLEXSPI, 200 * SECTOR_SIZE, (void *)&temp_cfg, sizeof(chip_cfg));
	EnableGlobalIRQ(intmask);
	
	if(camera_cfg.update == 1 || temp_cfg.stat != camera_cfg.stat){
		intmask=DisableGlobalIRQ();
		status = flexspi_nor_flash_erase_sector(FLASH_FLEXSPI, 200 * SECTOR_SIZE);
		EnableGlobalIRQ(intmask);
		
		intmask=DisableGlobalIRQ();
		status = flexspi_nor_flash_page_program(FLASH_FLEXSPI, 200 * SECTOR_SIZE, (void *)&camera_cfg,sizeof(chip_cfg));
		EnableGlobalIRQ(intmask);
	}else{
		camera_cfg = temp_cfg;		
	}	
}
void update_camera_cfg()
{
		uint32_t intmask = 0,status;

		intmask=DisableGlobalIRQ();
		status = flexspi_nor_flash_erase_sector(FLASH_FLEXSPI, 200 * SECTOR_SIZE);
		EnableGlobalIRQ(intmask);
		
		intmask=DisableGlobalIRQ();
		status = flexspi_nor_flash_page_program(FLASH_FLEXSPI, 200 * SECTOR_SIZE, (void *)&camera_cfg,sizeof(chip_cfg));
		EnableGlobalIRQ(intmask);
	
}