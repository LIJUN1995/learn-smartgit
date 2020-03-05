#ifndef _FINGERTECH_ALGORITHM_H
#define _FINGERTECH_ALGORITHM_H

extern void Navi_matResize(uint8_t* src, int32_t srcheight, int32_t srcwidth, uint8_t* dst, int32_t dstheight, int32_t dstwidth);
extern uint32_t coordinate(uint16_t *img, int height, int width,uint8_t *oricoor);
extern int QuarterPicture(uint8_t* source, int s_row, int s_column, uint8_t* result, int r_row, int r_column);
extern void FusionImg(unsigned short *src1,unsigned short* src2,int length, int num, int status);
extern void one_Extrect_three(unsigned char* src, unsigned char* dst, int height, int width,int num);
#endif