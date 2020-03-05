/********************************************************************************
*工 程 名:屏下指纹项目
*工程功能:配置项目各模块底层驱动
*时    间:2018.12.4
*版    本:v0.1
********************************************************************************/
#include "main.h"
//最后255的位置存基础信息
//0:1　升级标识
//2:5  升级的文件大小
//6    检验码
#define APPBASEINFO_ADDR  0x080FFF00
#define APPBASEINFO_LEN   7
//app　备份的地址
#define APPSAVE_ADDR       0x08080000 
#define ApplicationAddress 0x08004000

typedef  void (*pFunction)(void);
pFunction Jump_To_Application;
uint32_t JumpAddress;

#pragma pack(push)
#pragma pack(1)
typedef struct chip_cfg_t
{
	uint32_t x;
	uint32_t y;
	uint32_t stat;
	uint32_t update;
	uint32_t width;
	uint32_t hight;
	uint32_t exp_time;
	uint32_t full_size_w;  
	uint32_t update_program;
	uint32_t camera_id;
}chip_cfg;
#pragma pack(pop)

//升级时擦除的区域
uint32_t erase_secor[7]={CTL_SECTOR_NUMBER_1,CTL_SECTOR_NUMBER_2,CTL_SECTOR_NUMBER_3,CTL_SECTOR_NUMBER_4,CTL_SECTOR_NUMBER_5,CTL_SECTOR_NUMBER_6,CTL_SECTOR_NUMBER_7}; 
uint32_t erase_obwp[7]={OB_WP_1,OB_WP_2,OB_WP_3,OB_WP_4,OB_WP_5,OB_WP_6,OB_WP_7};

uint32_t erase_addr[7]={ADDR_FMC_SECTOR_1,ADDR_FMC_SECTOR_2,ADDR_FMC_SECTOR_3,ADDR_FMC_SECTOR_4,ADDR_FMC_SECTOR_5,ADDR_FMC_SECTOR_6,ADDR_FMC_SECTOR_7};

uint32_t count=0;
uint8_t  appbuf[256*1024];
uint32_t applen;
chip_cfg camera_cfg;
uint32_t size_w = 0;
int main(void)
{
	uint8_t fwinfobuf[APPBASEINFO_LEN];
	uint32_t num=0;
	uint16_t appdatanum=0;
	uint32_t appendaddr;
	uint32_t appoffset=0;
	uint32_t appsize=0;
	uint8_t ret =0;
	
	size_w = sizeof(chip_cfg)/sizeof(uint32_t);
	
	//初始化
	cdfinger_bspinit();
	//读升级信息
	fmc_read_8bit_data(APPBASEINFO_ADDR,APPBASEINFO_LEN,fwinfobuf);

	if((fwinfobuf[0]==0xa5)&&(fwinfobuf[1]==0x5a)){
		applen = ((fwinfobuf[2]<<24)|(fwinfobuf[3]<<16)|(fwinfobuf[4]<<8)|fwinfobuf[5]);     
		num=applen/128;
		for(count=0;count<num;count++){
			//读
			fmc_read_8bit_data((APPSAVE_ADDR+128*count),128,&appbuf[128*count]);
		}
		appdatanum = applen%128;
		if(appdatanum!=0){
			fmc_read_8bit_data((APPSAVE_ADDR+128*num),appdatanum,&appbuf[128*num]);
		}
		//计算验证
		ret = cdfinger_calccheckcode(&appbuf[0],applen);
		if(ret!=fwinfobuf[6]){
			printf("crc eroor\r\n");
			fmc_erase_sector(CTL_SECTOR_NUMBER_11);
			fmc_write_32bit_data(ADDR_FMC_SECTOR_11,size_w,(uint32_t *)&camera_cfg);
			//不升级  
			NVIC_SystemReset();
		}
			
		//开始搬运
		//擦除和写入
		appendaddr = ADDR_FMC_SECTOR_1 + applen;
		//更新代码
		num = cdfinger_calcearsenum(appendaddr); 
		appoffset = 0;
		
		for(count=0;count<num;count++){
			if(count<3){
				fmc_erase_sector(erase_secor[count]);
				ob_write_protection0_disable(erase_obwp[count]);
				fmc_write_8bit_data(erase_addr[count],0x4000,&appbuf[appoffset]);  
				appoffset += 0x4000;
			}else{
				fmc_erase_sector(erase_secor[count]);
				ob_write_protection0_disable(erase_obwp[count]);
				//fmc_write_8bit_data(erase_addr[count],0x10000,&appbuf[appoffset]);  
				fmc_write_32bit_data(erase_addr[count],0x4000,(uint32_t *)&appbuf[appoffset]);  
				appoffset += 0x10000;
			}
		}
		
		//擦除标识位
		fmc_erase_sector(CTL_SECTOR_NUMBER_11);

		//重启
		NVIC_SystemReset();
	}
	
	//跳转
	/* Jump to user application */
	JumpAddress = *(__IO uint32_t*) (ApplicationAddress + 4);
	Jump_To_Application = (pFunction) JumpAddress;
	/* Initialize user application's Stack Pointer */
	__set_MSP(*(__IO uint32_t*) ApplicationAddress);
	Jump_To_Application();
	while(1)
	{
	}
}
