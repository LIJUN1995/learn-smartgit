/*
 * The Clear BSD License
 * Copyright 2017 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 * that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "evkbimxrt1050_flexspi_nor_config.h"

/*******************************************************************************
 * Code
 ******************************************************************************/
#if defined(XIP_BOOT_HEADER_ENABLE) && (XIP_BOOT_HEADER_ENABLE == 1)
#if defined(__CC_ARM) || defined(__GNUC__)
__attribute__((section(".boot_hdr.conf")))
#elif defined(__ICCARM__)
#pragma location = ".boot_hdr.conf"
#endif

const flexspi_nor_config_t finggertechflash_config = {
    .memConfig =
        {
            .tag = FLEXSPI_CFG_BLK_TAG,//固定图
            .version = FLEXSPI_CFG_BLK_VERSION,//版本
            .readSampleClkSrc = kFlexSPIReadSampleClk_LoopbackInternally,//时钟来源
            .csHoldTime = 3u, //holdtime
            .csSetupTime = 3u, //setuptime
			.columnAddressWidth = 0u,//columnAdressWidth
			.deviceModeCfgEnable = true,
			.waitTimeCfgCommands = 0,
			.deviceModeType = 1,//Quad Enable command
			.deviceModeSeq.seqNum = 1,
			.deviceModeSeq.seqId = 4,
			.deviceModeArg = 0x000200,//set QE
			.deviceType = kFlexSpiDeviceType_SerialNOR,
            .sflashPadType = kSerialFlash_4Pads,
            .serialClkFreq = kFlexSpiSerialClk_133MHz,
            .sflashA1Size = 8u * 1024u * 1024u,
            .dataValidTime = {16u, 16u},
			.lookupTable =
			{
				//读数据操作
				//使用SDR模式标准SPI模式发送快速读命令0xEB,SDR模式 四线SPI模式发送 24位读地址
				[0]=FLEXSPI_LUT_SEQ(CMD_SDR,FLEXSPI_1PAD,0XEB,RADDR_SDR,FLEXSPI_4PAD,0x18),
				//dummy等待  4根线 6个时钟     读数据，四根线   后面的4可以忽略
				[1]=FLEXSPI_LUT_SEQ(DUMMY_SDR,FLEXSPI_4PAD,0X06,READ_SDR,FLEXSPI_4PAD,0x04),
				
				//x05 读状态寄存器  0x04 写禁止
				//标准SPI模式发送读状态寄存器 0x05  标准SPI模式读一个数据
				[1*4]=FLEXSPI_LUT_SEQ(CMD_SDR,FLEXSPI_1PAD,0x05,READ_SDR,FLEXSPI_1PAD,0x01),
				//0x06 写允许
				//标准SPI模式写允许寄存器 0x06,后面不需要在发送命令发送停止命令
				[3*4]=FLEXSPI_LUT_SEQ(CMD_SDR,FLEXSPI_1PAD,0x06,STOP,0,0),
				
				//擦除4K扇区指令,标准SPI模式发送擦除扇区指令0x20 ,发送24位地址
				[5*4]=FLEXSPI_LUT_SEQ(CMD_SDR,FLEXSPI_1PAD,0x20,RADDR_SDR,FLEXSPI_1PAD,0x18),
				
				//写页命令 标准SPI模式发送写页指令 0x02   发送需要操作的24位地址
				[9*4]= FLEXSPI_LUT_SEQ(CMD_SDR,FLEXSPI_1PAD,0x02,RADDR_SDR,FLEXSPI_1PAD,0x18),
				//写入发送的数据，数据个数无所谓
				[9*4+1]=FLEXSPI_LUT_SEQ(WRITE_SDR,FLEXSPI_1PAD,0x02,STOP,0,0),
				
				//擦除整片flash
				[11*4] = FLEXSPI_LUT_SEQ(CMD_SDR,FLEXSPI_1PAD,0x60,STOP,0,0),
				//dummy 设置
				[15*4] = FLEXSPI_LUT_SEQ(DUMMY_SDR,FLEXSPI_1PAD,0xff,STOP,0,0),
			},
        },
    .pageSize = 256u,
    .sectorSize = 4u * 1024u,
    .ipcmdSerialClkFreq = 1,
    .isUniformBlockSize = false,
};
#endif /* XIP_BOOT_HEADER_ENABLE */
