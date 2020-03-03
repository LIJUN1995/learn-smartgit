#ifndef COMMON_H_
#define COMMON_H_

#include <stdio.h>
#include "stdint.h"

#define __DEBUG__
#ifdef __DEBUG__
	#define DEBUG(format,...) printf("File: "__FILE__", Line: %05d: "format"\n", __LINE__, ##__VA_ARGS__)
#else  
	#define DEBUG(format,...)  
#endif 


int fputc(int ch, FILE *f);
void cdfinger_usart_init(void);
void cdfinger_bspinit(void);
uint8_t cdfinger_calccheckcode(uint8_t* buf,uint32_t len);
uint8_t cdfinger_calcearsenum(uint32_t addr);

#endif

