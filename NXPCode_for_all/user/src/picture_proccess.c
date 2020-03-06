/******************************************************************************
*文 件 名：picture_process
*文件功能：对图像进行各种处理
*时    间：2018.10.15
******************************************************************************/
#include "picture_proccess.h"
#include "fsl_lpuart.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define DEBUG_PRINT 0
/******************************************************************************
*函 数 名：CompressionPicture
*函数功能：先将yuv422格式大小为240*320的数组剪切成240*240（每像素两个字节）,在田
*          字格布局的四个像素取平均y值存进一字节大小的数据中变成120*120（每像素
*          一个字节）
*输    入：source		:待处理图像数组的地址
*          s_row		:待处理图像数组的行数
*          s_column	:待处理图像数组的列数
*					 result		:结果图数组地址
*          r_row		:结果图的行数
*          r_column	:结果图的列数
*          x				:中心点横坐标（处于第几列）
*          y				:中心点纵坐标（处于第几行）
*输    出：成功返回零，无效的数组地址返回-1，原始数据大小行列中有小于结果图返回-2
*全局变量：
*备    注：
******************************************************************************/
int CompressionPicture(uint8_t* source, int s_row, int s_column, char* result, int r_row, int r_column, int x, int y){
	int i, j;
	uint8_t pic[320*240];\
	
	/*错误输入排除*/
	if (!source){
		printf("give invalid address!\n");
		return -1;
	}
	if (s_row < r_row || s_column < r_column){
		printf("the picture to be cut must be bigger than result!\n");
		return -2;
	}
	if (!pic){
		printf("malloc pic error!\n");
		return -3;
	}
	if(x+r_row/2>s_row || x-r_row/2<0 || y+r_column/2>s_column || y-r_column/2<0){
		printf("wrong (x,y),Crossing!\n");
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
	
	/*提取灰度图*/
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
	
	/*将原始数据图裁剪成目标大小*/
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

	/*4合一算法压缩数据*/
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
*函 数 名：GetOnlyY
*函数功能：输出灰度图
*输    入：pic		:待处理图像的数组地址
*          cloumn	:宽
*          row		:高
*          result	:结果图数组地址
*输    出：
*全局变量：
*备    注：
******************************************************************************/
int GetOnlyY(uint8_t* pic, int column, int row, uint8_t* result){
	int i;
	if(pic == NULL){
		printf("pic = NULL!\n");
		return -1;
	}
	for(i=0; i<row*column; i++){
		*(result+i) = *(pic+i*2);	
	}
	return 0;
}

/******************************************************************************
*函 数 名：CutPicture
*函数功能：根据中心坐标点和裁剪行列宽度将原图剪裁出图
*输    入：pic			:图像数组地址
*          s_column	:待处理图像数组的列数
*          s_row		:待处理图像数组的行数
*          x				:中心坐标x
*          y				:中心坐标y
*          column		:剪切后的图像的列长
*          row			:剪切后的图像的行长
*输    出：
*全局变量：
*备    注：这个函数是在输入图像数组的数据上做更改，这将覆盖原始数据
******************************************************************************/
int CutPicture(uint8_t* pic, int s_column, int s_row, int x, int y, int column, int row){
	int i;
	if(!pic){
		printf("pic = NULL!\n");
		return -1;
	}
	if(x+column/2>s_column || x-column/2<0 || y+row/2>s_row || y-row/2<0){
		printf("wrong (x,y),Crossing!\n");
		return -2;
	}
	for (i = 0; i < row; i++){
		memcpy(pic+i*column, pic+(y+i-row/2)*s_column+x-column/2, column);	
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
int QuarterPicture(uint8_t* source, int s_row, int s_column, uint8_t* result, int r_row, int r_column){
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

