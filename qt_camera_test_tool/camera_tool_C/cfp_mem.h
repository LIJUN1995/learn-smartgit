/*************************************************************************
        > File Name: fp_mem.h
        > Author:fpw
        > Mail:
        > Created Time: 2019年09月19日 星期四 02时25分42秒
 ************************************************************************/

#ifndef _FP_MEM_H
#define _FP_MEM_H
#include "stdint.h"

void *cfp_malloc(uint32_t);
void *cfp_memcpy(void *, const void *, uint32_t);
void *cfp_memset(void *, int32_t, uint32_t);
void cfp_free(void *);

#endif
