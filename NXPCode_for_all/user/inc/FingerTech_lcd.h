#ifndef _FINGERTECH_LCD_H
#define _FINGERTECH_LCD_H

#define APP_IMG_HEIGHT 320
#define APP_IMG_WIDTH 240
#define APP_HSW 41
#define APP_HFP 4
#define APP_HBP 8
#define APP_VSW 10
#define APP_VFP 4
#define APP_VBP 2

#define RGB(R,G,B)	(((R >> 3) << 11) | ((G >> 2) << 5) | (B >> 3))	
#define RGB565_R(x)  ((x >> 8) & 0xF8)
#define RGB565_G(x)  ((x >> 3) & 0xFC)
#define RGB565_B(x)  ((x << 3) & 0xF8)

void Fingertech_LCDPinInit(void);
void Fingertech_LCDBKon(void);
void Fingertech_InitLcdifPixelClock(void);
void Fingertech_ELCDIFInit(void);
void Fingertech_dispalybmp(uint16_t frameBuffer[APP_IMG_HEIGHT][APP_IMG_WIDTH],const unsigned char* bmpbuf);
void FingerTech_displaycameraimg(const unsigned char* imgbuf);

#endif

