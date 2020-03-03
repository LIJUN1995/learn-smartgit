#include "main.h"
#include "FingerTech_camera.h"

#define  Up_amplify 20
/**************************************
* @brief Scale or expand image data
*	
*	@param	source data 
*	@param	sourec heigth
*	@param	source width
*	@param	Destination data
*	@param	Destination heigth
* @param	Destination width
*
*	@return
****************************************/
void Navi_matResize(uint8_t* src, int32_t srcheight, int32_t srcwidth, uint8_t* dst, int32_t dstheight, int32_t dstwidth)
{
	uint8_t* dataDst = (uint8_t*)dst;
	int32_t stepDst = dstwidth;
	uint8_t* dataSrc = (uint8_t*)src;
	int32_t stepSrc = srcwidth;
	int32_t iWidthSrc = srcwidth;
	int32_t iHiehgtSrc = srcheight;
	int32_t i, j;
	int64_t sx, sy;
	//int  scale_y = srcheight*(1<<(Up_amplify-1))/dstheight;//0.5;
	//int  scale_x =  srcwidth*(1<<(Up_amplify-1))/dstwidth;//0.5;
	int32_t  scale_y = srcheight * 1000 / dstheight;//;
	int32_t  scale_x = srcwidth * 1000 / dstwidth;//;
	int64_t fx, fy;
#if Up_amplify > 8
	int64_t cbufy[2];
	int64_t cbufx[2];
#else
	uint32_t cbufx[2];
	uint32_t cbufy[2];
#endif

	for (j = 0; j < dstheight; ++j)
	{
		fy = ((j << 1) + 1)*scale_y - 1000;
		fy = ((fy << (Up_amplify - 1)) / 1000);
		sy = fy >> (Up_amplify);
		fy -= (sy << Up_amplify);

		sy = sy < (iHiehgtSrc - 2) ? sy : (iHiehgtSrc - 2);
		sy = 0 > sy ? 0 : sy;

		cbufy[0] = (1 << Up_amplify) - fy;// cv::saturate_cast<short>((1.f - fy) * 2048);  
		cbufy[1] = fy;//2048 - cbufy[0];  

		for (i = 0; i < dstwidth; ++i)
		{
			fx = ((i << 1) + 1)*scale_x - 1000;
			fx = ((fx << (Up_amplify - 1)) / 1000);
			sx = fx >> (Up_amplify);
			fx -= (sx << Up_amplify);

			if (sx < 0) {
				fx = 0, sx = 0;
			}
			if (sx >= iWidthSrc - 1) {
				fx = 0, sx = iWidthSrc - 2;
			}
			cbufx[0] = (1 << Up_amplify) - fx;// cv::saturate_cast<short>((1.f - fx) * 2048);  
			cbufx[1] = fx;// 2048 - cbufx[0];  
			*(dataDst + j*stepDst + i) = (uint8_t)((*(dataSrc + sy*stepSrc + sx) * cbufx[0] * cbufy[0] +
				*(dataSrc + (sy + 1)*stepSrc + sx) * cbufx[0] * cbufy[1] +
				*(dataSrc + sy*stepSrc + (sx + 1)) * cbufx[1] * cbufy[0] +
				*(dataSrc + (sy + 1)*stepSrc + (sx + 1)) * cbufx[1] * cbufy[1] + (1LL << (Up_amplify * 2 - 1))) >> (Up_amplify * 2));
		}
	}
}


float im_mean(uint16_t *img, int height, int width)
{
	int i;
	float mean = 0;
	for (i = 0; i < height*width; i++)
	{
		mean = mean + *(img + i);
	}
	mean = mean / (height*width);
	return mean;
}

uint32_t coordinate(uint16_t *img, int height, int width, uint8_t *oricoor)
{
	int coor[4] = { 0 };
	int i, j;
	unsigned int x, y;
	x = oricoor[0] * 256 + oricoor[1];
	y = oricoor[2] * 256 + oricoor[3];
	float mean = im_mean(img, height, width);
	float t = mean / 60;
	int sum_x1 = 0, sum_x2 = 0, sum_y1 = 0, sum_y2=0,area1=0,area2=0;
//	for (i = 0; i < height*width; i++)
//	{
//		if (*(img + i) >= 40 * t)
//			*(img + i) = 255;
//		else
//			*(img + i) = 0;
//	}

	for (i = 0; i < height; i++)
		for (j = 0; j < width; j++)
		{
			if (*(img + i*width + j)>= 40 * t)
			{
				sum_x1 = sum_x1 + i;
				sum_y1 = sum_y1 + j;
				area1 = area1 + 1;
			}
			else
			{
				sum_x2 = sum_x2 + i;
				sum_y2 = sum_y2 + j;
				area2 = area2 + 1;
			}
			
		}
	coor[0] = sum_x1 / area1;
	coor[1] = sum_y1 / area1;
	coor[2] = sum_x2 / area2;
	coor[3] = sum_y2 / area2;
	if (abs(coor[0] - height / 2) > 5 || abs(coor[1] - width / 2) > 5)
	{
		x = x + (coor[0] - height / 2);
		y = y + (coor[1] - width / 2);
		oricoor[0]=x>>8;
		oricoor[1] = x - oricoor[0]*256;
		oricoor[2] = y >> 8;
		oricoor[3] = y - oricoor[2]*256;
		
		return 1;
	}
	
	return 0;
}

/******************************************************************************
*函 数 名：QuarterPicture
*函数功能：根据四个一算法压缩图像
*输    入：pic			:图像数组地址
*          column		:原始图像的列长
*          row			:原始图像的行长
*输    出：
*全局变量：
*备    注：这个函数是在输入图像数组的数据上做更改，这将覆盖原始数据
******************************************************************************/
int  QuarterPicture(uint8_t* source, int s_row, int s_column, uint8_t* result, int r_row, int r_column)
{
	int i, j; 
	if(!source){
		printf("source = NULL!\n");
		return -1;
	}

	for (i=0; i<r_row*2; i+=2){
		for (j=0; j<r_column*2; j+=2){
			*(result+i/2*r_column+j/2) = (*(source+i*s_column+j)+*(source+i*s_column+j+1)+*(source+(i+1)*s_column+j)+*(source+(i+1)*s_column+j+1))/4;
		}
	}
	
	return 0;
}

void one_Extrect_three(unsigned char* src, unsigned char* dst, int height, int width,int num)
{
	int X1,X2,X3;
	int y = 0;
	int i, j, index = 0;
	unsigned char *a1,*a2,*a3;
	unsigned char *b;
	static uint8_t count = 1;
	
	X1=(camera_cfg.x1-M_W/2);
	X2=(camera_cfg.x2-M_W/2);
	X3=(camera_cfg.x3-M_W/2);
	
	if(num == 1)
		count = 1;
	for (i = y; i < y + height; i++)
	{
		a1 = dst + (i - y) * width;
		a2 = a1 + height * width ;
		a3 = a2 + height * width ;
		b = src + i * camera_cfg.width;
		for (j = X1; j < X1 + width; j++)
		{
			*(a1 + j - X1) = ((*(a1 + j - X1))*count+((*(b + j)>>2 | *(b + j)<<6)&0x00ff))/num;
		}
		for (j = X2; j < X2 + width; j++)
		{
			*(a2 + j - X2) = ((*(a2 + j - X2))*count+((*(b + j)>>2 | *(b + j)<<6)&0x00ff))/num;
		}
		for (j = X3; j < X3 + width; j++)
		{
			*(a3 + j - X3) = ((*(a3 + j - X3))*count+((*(b + j)>>2 | *(b + j)<<6)&0x00ff))/num;
		}
	}
	count = num;
}

void FusionImg(unsigned short *src1,unsigned short* src2,int length, int num, int status)
{
	int i;
	
#ifdef CAMERA_10BIT
	if(status){
		for (i = 0; i < length; i++){
			*(src1 + i) = *(src1 + i) + *(src2 + i);
		}
	}else{
		for (i = 0; i < length; i++){
			*(src1 + i) = (*(src1 + i)+ *(src2 + i))/num;
		}
	}
#else
	if(status){
		for (i = 0; i < length; i++){
			*(src1 + i) = *(src1 + i) + ((*(src2 + i))&0x0fff);
		}
	}else{
		for (i = 0; i < length; i++){
			*(src1 + i) = (*(src1 + i)+ ((*(src2 + i))&0x0fff))/num;
		}
	}
#endif
}