/******************************************************************************
*�� �� ����picture_process
*�ļ����ܣ���ͼ����и��ִ���
*ʱ    �䣺2018.10.15
******************************************************************************/
#include "picture_proccess.h"
#include "fsl_lpuart.h"
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
	
	/*��ԭʼ����ͼ�ü���Ŀ���С*/
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
		printf("pic = NULL!\n");
		return -1;
	}
	for(i=0; i<row*column; i++){
		*(result+i) = *(pic+i*2);	
	}
	return 0;
}

/******************************************************************************
*�� �� ����CutPicture
*�������ܣ��������������Ͳü����п�Ƚ�ԭͼ���ó�ͼ
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

