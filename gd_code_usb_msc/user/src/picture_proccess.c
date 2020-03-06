/******************************************************************************
*�� �� ����picture_process
*�ļ����ܣ���ͼ����и��ִ���
*ʱ    �䣺2018.10.15
******************************************************************************/
#include "picture_proccess.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define DEBUG_PRINT 0
/******************************************************************************
*�� �� ����CompressionPicture
*�������ܣ��Ƚ�yuv422��ʽ��СΪ240*320��������г�240*240��ÿ���������ֽڣ�,����
*          �ָ񲼾ֵ��ĸ�����ȡƽ��yֵ���һ�ֽڴ�С�������б��120*120��ÿ����
*          һ���ֽڣ�
*��    �룺source		:������ͼ������ĵ�ַ
*          s_row		:������ͼ�����������
*          s_column	:������ͼ�����������
*					 result		:���ͼ�����ַ
*          r_row		:���ͼ������
*          r_column	:���ͼ������
*          x				:���ĵ�����꣨���ڵڼ��У�
*          y				:���ĵ������꣨���ڵڼ��У�
*��    �����ɹ������㣬��Ч�������ַ����-1��ԭʼ���ݴ�С��������С�ڽ��ͼ����-2
*ȫ�ֱ�����
*��    ע��
******************************************************************************/
int CompressionPicture(uint8_t* source, int s_row, int s_column, char* result, int r_row, int r_column, int x, int y){
	int i, j;
	uint8_t pic[320*240];\
	
	/*���������ų�*/
	if (!source){
#ifdef SPI_TRANSFER
		printf("give invalid address!\n");
#endif
		return -1;
	}
	if (s_row < r_row || s_column < r_column){
#ifdef SPI_TRANSFER
		printf("the picture to be cut must be bigger than result!\n");
#endif
		return -2;
	}
	if (!pic){
#ifdef SPI_TRANSFER
		printf("malloc pic error!\n");
#endif
		return -3;
	}
	if(x+r_row/2>s_row || x-r_row/2<0 || y+r_column/2>s_column || y-r_column/2<0){
#ifdef SPI_TRANSFER
		printf("wrong (x,y),Crossing!\n");
#endif
		return -4;
	}
#if DEBUG_PRINT
	for (i=0; i<s_row; i++){
		for (j=0; j<s_column*2; j++){
			printf("%02x ", *(source+i*s_column*2+j));
		}
		printf("\r");
	}
	printf("\n\n\n\n");
#endif	
	
	/*��ȡ�Ҷ�ͼ*/
	for (i = 0; i < s_row*s_column; i++){
		memcpy(pic+i,source+2*i,1);
	}
	//LPUART_WriteBlocking(LPUART1,(uint8_t*)pic,s_row*s_column);
#if DEBUG_PRINT
	for (i=0; i<s_row; i++){
		for (j=0; j<s_column; j++){
			printf("%02x ", *(source+i*s_column+j));
		}
		printf("\r");
	}
	printf("\n\n\n\n");
#endif		
	
	/*��ԭʼ����ͼ�ü��Ŀ���С*/
	for (i = 0; i < r_row*2; i++){
		memcpy(pic+i*r_column*2, pic+(y+i-r_row)*r_column*2+x-r_column, r_column*2);	
	}
	//LPUART_WriteBlocking(LPUART1,(uint8_t*)pic,r_row*r_column*4);
#if DEBUG_PRINT
	for (i=0; i<r_row*2; i++){
		for (j=0; j<r_column*2; j++){
			printf("%02x ", *(pic+i*r_column+j));
		}
		printf("\r");
	}
	printf("\n\n\n\n");
#endif		

	/*4��һ�㷨ѹ������*/
	for (i=0; i<r_row*2; i+=2){
		for (j=0; j<r_column*2; j+=2){
			*(result+i/2*r_column+j/2) = (*(pic+i*s_column+j)+*(pic+i*s_column+j+1)+*(pic+(i+1)*s_column+j)+*(pic+(i+1)*s_column+j+1))/4;
		}
	}
	//LPUART_WriteBlocking(LPUART1,(uint8_t*)result,r_row*r_column);
#if DEBUG_PRINT
	for (i=0; i<r_row; i++){
		for (j=0; j<r_column; j++){
			printf("%02x ", *(result+i*r_column+j));
		}
		printf("\r");
	}
	printf("\n\n\n\n");
#endif
	return 0;
}

/******************************************************************************
*�� �� ����GetOnlyY
*�������ܣ�����Ҷ�ͼ
*��    �룺pic		:������ͼ��������ַ
*          cloumn	:��
*          row		:��
*          result	:���ͼ�����ַ
*��    ����
*ȫ�ֱ�����
*��    ע��
******************************************************************************/
int GetOnlyY(uint8_t* pic, int column, int row, uint8_t* result){
	int i;
	if(pic == NULL){
#ifdef SPI_TRANSFER
		printf("pic = NULL!\n");
#endif
		return -1;
	}
	for(i=0; i<row*column; i++){
		*(result+i) = *(pic+i*2);	
	}
	return 0;
}

/******************************************************************************
*�� �� ����CutPicture
*�������ܣ��������������Ͳü����п�Ƚ�ԭͼ��ó�ͼ
*��    �룺pic			:ͼ�������ַ
*          s_column	:������ͼ�����������
*          s_row		:������ͼ�����������
*          x				:��������x
*          y				:��������y
*          column		:���к��ͼ����г�
*          row			:���к��ͼ����г�
*��    ����
*ȫ�ֱ�����
*��    ע�����������������ͼ������������������ģ��⽫����ԭʼ����
******************************************************************************/
int CutPicture(uint8_t* pic, int s_column, int s_row, int x, int y, int column, int row){
	int i;
	if(!pic){
#ifdef SPI_TRANSFER
		printf("pic = NULL!\n");
#endif
		return -1;
	}
	if(x+column/2>s_column || x-column/2<0 || y+row/2>s_row || y-row/2<0){
#ifdef SPI_TRANSFER
		printf("wrong (x,y),Crossing!\n");
#endif
		return -2;
	}
	for (i = 0; i < row; i++){
		memcpy(pic+i*column, pic+(y+i-row/2)*s_column+x-column/2, column);	
	}
	return 0;
}

/******************************************************************************
*�� �� ����QuarterPicture
*�������ܣ������ĸ�һ�㷨ѹ��ͼ��
*��    �룺pic			:ͼ�������ַ
*          column		:ԭʼͼ����г�
*          row			:ԭʼͼ����г�
*��    ����
*ȫ�ֱ�����
*��    ע�����������������ͼ������������������ģ��⽫����ԭʼ����
******************************************************************************/
int QuarterPicture(uint8_t* source, int s_row, int s_column, uint8_t* result, int r_row, int r_column){
	int i, j; 
	if(!source){
#ifdef SPI_TRANSFER
		printf("source = NULL!\n");
#endif
		return -1;
	}
//	for (i = 0; i < row/2; i++){
//		for (j = 0;j < column/2; j++){
//			*(pic+i*column/2+j) = (*(pic+i*2*column+j*2)+*(pic+i*2*column+j*2+1)+*(pic+(i*2+1)*column+j*2)+*(pic+(i*2+1)*column+j*2+1))/4;
//		}
//	}
	

	for (i=0; i<r_row*2; i+=2){
		for (j=0; j<r_column*2; j+=2){
			*(result+i/2*r_column+j/2) = (*(source+i*s_column+j)+*(source+i*s_column+j+1)+*(source+(i+1)*s_column+j)+*(source+(i+1)*s_column+j+1))/4;
		}
	}
	
	return 0;
}

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

void Sum_Img(int* src1, unsigned char* src2, int length)
{
	int i;
	for (i = 0; i < length; i++)
	{
		*(src1 + i) = *(src1 + i) + *(src2 + i);
	}
}



void Mean_Img(int* src, unsigned char* dst, int num, int length)
{
	int i;
	for (i = 0; i < length; i++)
	{
		*(dst + i) = (unsigned char)(*(src + i) / num);
	}
}