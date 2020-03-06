#ifndef PICTURE_PROCESS_H
#define PICTURE_PROCESS_H
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
int CompressionPicture(uint8_t* source, int s_row, int s_column, char* result, int r_row, int r_column, int x, int y);
int GetOnlyY(uint8_t* pic, int column, int row, uint8_t* result);
int CutPicture(uint8_t* pic, int s_column, int s_row, int x, int y, int column, int row);
int QuarterPicture(uint8_t* source, int s_row, int s_column,uint8_t* result, int r_row, int r_column);

#endif
