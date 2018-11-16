/* ************************************************************************
 *       Filename:  test.c
 *    Description:  
 *        Version:  1.0
 *        Created:  2018年09月25日 09时41分45秒
 *       Revision:  none
 *       Compiler:  gcc
 *         Author:  YOUR NAME (), 
 *        Company:  
 * ************************************************************************/



/*************************************************************************
	> File Name: test.c
	> Author: ma6174
	> Mail: ma6174@163.com 
	> Created Time: 2018年09月25日 星期二 09时41分45秒
 ************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include "glimmerAl.h"
#include "preprocess.h"

typedef unsigned char uint8_t;
typedef unsigned int    uint32_t;

extern int fb_main(void);
extern void draw_image(int w,int h,void* image,void* finger);
extern void clear_screen(int w, int h, void* _loc);
extern void Navi_matResize(uint8_t* src, int32_t srcheight, int32_t srcwidth, uint8_t* dst, int32_t dstheight, int32_t dstwidth);

#define CDFINGER_IOCTL_MAGIC_NO			0xFB
#define CDFINGER_SET_SPI_SPEED			_IOW(CDFINGER_IOCTL_MAGIC_NO, 7, uint8_t)
#define CDFINGER_GET_SCREEN_STATUS		_IOW(CDFINGER_IOCTL_MAGIC_NO,29,int)
#define CDFINGER_CTL_SPI				_IOW(CDFINGER_IOCTL_MAGIC_NO,30,cdfinger_spi_data)
#define CDFINGER_GETIMAGE				_IOW(CDFINGER_IOCTL_MAGIC_NO, 1, uint8_t)
#define BUF_SIZE_R						128*128
#define BUF_SIZE_T						8

typedef struct _cdfinger_spi_data {
	unsigned char *tx;
	unsigned char *rx; 
	int length;
}cdfinger_spi_data;

enum spi_speed {
	CDFINGER_SPI_4M1 = 1,
	CDFINGER_SPI_4M4,
	CDFINGER_SPI_4M7,
	CDFINGER_SPI_5M1,
	CDFINGER_SPI_5M5,
	CDFINGER_SPI_6M1,
	CDFINGER_SPI_6M7,
	CDFINGER_SPI_7M4,
	CDFINGER_SPI_8M
};

/*proprecess start*/
#define SENSOR_WIDTH				128
#define SENSOR_HEIGHT				128
#define SENSOR_BYTES_PERPIXEL		8
#define SENSOR_BUFFER_LENGTH		(SENSOR_WIDTH * SENSOR_HEIGHT * SENSOR_BYTES_PERPIXEL)
#define IS_COATING					0x00000000
#define IS_FLOTING					0x00000001
#define IS_PIXEL_CANCEL				0x00000000
#define THR_SELECT_BMP				700

typedef struct _cfp_bgImg_t{
    pthread_mutex_t lock;
    int isready;                                /* < is background image ready */
    int calType;
    calibrationPara caliPara;
    uint8_t buffer[SENSOR_BUFFER_LENGTH];       /* < background image buffer */
} cfp_bgImg_t;

REGISTER_STANDARD *gRegisterSession = NULL;		
cfp_bgImg_t gBgImg = { 0 };
/*proprecess end*/

int fp_SaveGrayBitmap(const char *FilePath, unsigned char *pData, int row, int colume)
{
	/*Check parameter*/
	FILE *fd;
	unsigned char grayBitmapHeader[1078] = {0};
	unsigned char pad[4] = {0};
	int colume_t = 0;
	int row_t = 0;
	int i = 0;
	if (NULL == pData)
	{
		return -1;
	}
	fd = fopen(FilePath, "wb+");
	if (fd == NULL)
	{
		printf("\nthe file open error\n");
		return -1;
	}
	/*Gray bitmap file header*/
	{
		colume_t = (colume + 3) & 0xFC;
		row_t = (row + 3) & 0xFC;
		grayBitmapHeader[0] = 0x42;
		grayBitmapHeader[1] = 0x4d;
		grayBitmapHeader[2] = 0x36;
		grayBitmapHeader[3] = 0x28;
		grayBitmapHeader[4] = 0x00;
		grayBitmapHeader[5] = 0x00;
		grayBitmapHeader[6] = 0x00;
		grayBitmapHeader[7] = 0x00;
		grayBitmapHeader[8] = 0x00;
		grayBitmapHeader[9] = 0x00;
		grayBitmapHeader[10] = 0x36;
		grayBitmapHeader[11] = 0x04;
		grayBitmapHeader[12] = 0x00;
		grayBitmapHeader[13] = 0x00;
		grayBitmapHeader[14] = 0x28;
		grayBitmapHeader[15] = 0x00;
		grayBitmapHeader[16] = 0x00;
		grayBitmapHeader[17] = 0x00;

		grayBitmapHeader[18] = (colume_t & 0xFF);
		grayBitmapHeader[19] = (colume_t >> 8 & 0xFF);
		grayBitmapHeader[20] = (colume_t >> 16 & 0xFF);
		grayBitmapHeader[21] = (colume_t >> 24 & 0xFF);

		grayBitmapHeader[22] = (row_t & 0xFF);
		grayBitmapHeader[23] = (row_t >> 8 & 0xFF);
		grayBitmapHeader[24] = (row_t >> 16 & 0xFF);
		grayBitmapHeader[25] = (row_t >> 24 & 0xFF);

		grayBitmapHeader[26] = 0x01;
		grayBitmapHeader[27] = 0x00;

		grayBitmapHeader[28] = 0x08;
		grayBitmapHeader[29] = 0x00;

		for (i = 0; i < 256; i++)
		{
		grayBitmapHeader[54 + i * 4] = i;
		grayBitmapHeader[54 + i * 4 + 1] = i;
		grayBitmapHeader[54 + i * 4 + 2] = i;
		}
		int ret = fwrite((char *)grayBitmapHeader,1 , 1078, fd);
		/*Be careful , bitmap save turn is buttom to top*/
		for (i = 0; i < row; i++)
		{
		ret = fwrite((char *)(pData + (row - i - 1) * colume), 1, colume, fd);
		
		if (colume_t > colume)
			ret = fwrite((char *)pad, 1, colume_t - colume, fd); 
		
		}
		fseek(fd,0,SEEK_SET);
		fseek(fd,0,SEEK_END);
		long longBytes=ftell(fd);// longBytes就是文件的长度
		printf("longBytes=%ld\n",longBytes);
		fclose(fd);
	}
	return 0;
	}

int fp_SaveGrayBmpFile(const char *FilePath, unsigned char *pData,
			int row, int colume, int bitWide)
{
	int i;
	int data_len;
	data_len = row * colume;

	if (bitWide == 8)
	{
		return fp_SaveGrayBitmap(FilePath, pData, row, colume);
	}

	if (bitWide == 12)
	{
		unsigned char tmp_data[128 * 128];
		if (data_len > sizeof(tmp_data))
		data_len = sizeof(tmp_data);
		unsigned short *iData = (unsigned short *)pData;
		for (i = 0; i < data_len; i++)
		{
		tmp_data[i] = (((pData[2 * i] >> 4) & 0x0F) | ((pData[2 * i + 1] << 4) & 0xF0)) & 0xFF;
		//tmp_data[i] = iData[i]/16;
		}
		return fp_SaveGrayBitmap(FilePath, tmp_data, row, colume);
	}
	return -1;
}

int fp_SaveBinFile(const char *FilePath, unsigned char *pData,int row, int colume)
{
	FILE *fb = NULL;
	fb = fopen(FilePath,"wb+");
	if(fb == NULL){
		printf("open failed\n");
		return -1;
	}
	fwrite(pData,row,colume,fb);
	fclose(fb);

	return 0;
}

int main(int argc,char *argv[])
{
	int m_fd = -1,count = 0,num = 0,r_num = 0,i,ret;
	cdfinger_spi_data data;
	struct timeval tp,te;
	char file_name[64] = {0};

	if(argc != 3){
		printf("the number of parameter is errored.  please input three parameters!\n");
		return -1;
	}

	m_fd = open("/dev/fpsdev0",O_RDWR);
	if(m_fd < 0) {
		printf("open device failed!\n");
		return m_fd;
	}
	ret = ioctl(m_fd,CDFINGER_SET_SPI_SPEED,CDFINGER_SPI_4M1);
	ret = fb_main();

	data.tx = (unsigned char *)malloc(BUF_SIZE_T);
	data.rx = (unsigned char *)malloc(atoi(argv[1])*atoi(argv[2]));
	memset(data.tx,0x90,BUF_SIZE_T);
	memset(data.rx,0x66,atoi(argv[1])*atoi(argv[2]));
	data.length = atoi(argv[1])*atoi(argv[2]);
	
	unsigned char *bg_buff = (unsigned char *)malloc(atoi(argv[1])*atoi(argv[2]));
	uint16_t*image_buff = (uint16_t*)malloc(sizeof(uint16_t)*atoi(argv[1])*atoi(argv[2]));
	
	/*proprecess start*/
	memset(&gBgImg, 0, sizeof(cfp_bgImg_t));
	gRegisterSession = beginRegister();
	gRegisterSession->nSchedule = 0;
	gRegisterSession->nUsedNum = 0;
	int fd_bg = open("/data/picture/bg", O_RDWR);
	ret = read(fd_bg, bg_buff, atoi(argv[1])*atoi(argv[2])); 
	//ret = fp_SaveGrayBmpFile("/data/picture/fd_bg.bmp", bg_buff, atoi(argv[1]), atoi(argv[2]), 8);
	for(i = 0; i< atoi(argv[1])*atoi(argv[2]); i++){
		image_buff[i] = bg_buff[i]*16;
	}
	printf("22222222222222222222\n");
	gBgImg.calType = 0;
	preProcess((uint16_t*)image_buff,atoi(argv[1])*atoi(argv[2]), &(gBgImg.caliPara), IS_PIXEL_CANCEL | IS_FLOTING | IS_COATING, &(gBgImg.calType), THR_SELECT_BMP);
	gBgImg.calType = 1;
	gBgImg.isready = 1;
	printf("3333333333333\n");
	close(fd_bg);
	/*proprecess end*/

	do{
		if(count == 0){
			printf("How much pictures do you want to save:");
			scanf("%d",&count);
			if(count == 999)
				break;
		}
	
		ret = ioctl(m_fd,CDFINGER_CTL_SPI,&data);
		for(i = 0; i< atoi(argv[1])*atoi(argv[2]); i++){
			image_buff[i] = data.rx[i]*16;
		}

		gettimeofday(&tp,NULL);
		printf("111111111111111111\n");
		/*proprecess start*/			
		preProcess((uint16_t*)image_buff,atoi(argv[1])*atoi(argv[2]), &gBgImg.caliPara,IS_PIXEL_CANCEL | IS_FLOTING | IS_COATING, &gBgImg.calType, THR_SELECT_BMP);
		sprintf(file_name, "/data/picture/finger%d.bmp", num);
		ret = fp_SaveGrayBmpFile(file_name, (unsigned char *)gBgImg.caliPara.dataBmp,  atoi(argv[1]),atoi(argv[2]), 8);	
		/*proprecess end*/
		draw_image(atoi(argv[1]),atoi(argv[2]),data.rx,gBgImg.caliPara.dataBmp);//mx
		gettimeofday(&te,NULL);
		printf("draw_image spend %ld ms,save successed ========== %d\n",te.tv_usec-tp.tv_usec,num);

		//sprintf(file_name, "/data/picture/%d.bmp", num);
		//fp_SaveGrayBmpFile(file_name, mx,atoi(argv[1]),atoi(argv[2]),8);
		//sprintf(file_name, "/data/picture/%d.bin", num);
		//fp_SaveBinFile(file_name,mx,atoi(argv[1]),atoi(argv[2]));
		num++;
		count--;
	}while(count>=0);
	// free(image_buff);
	// free(bg_buff);
	// free(data.rx);
	// free(data.tx);
	close(m_fd);
	return 0;
}
