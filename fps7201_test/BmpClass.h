#ifndef __BMP_CLASS_H__
#define __BMP_CLASS_H__

#pragma pack(2)

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef int LONG;

typedef struct {
    WORD bfType;//文件类型，必须是0x424D，即字符“BM”
	DWORD bfSize;//文件大小
	WORD bfReserved1;//保留字
	WORD bfReserved2;//保留字
	DWORD bfOffBits;//从文件头到实际位图数据的偏移字节数
} BITMAPFILEHEADER;

typedef struct {	
    DWORD biSize;//信息头大小	
    LONG biWidth;//图像宽度	
    LONG biHeight;//图像高度	
    WORD biPlanes;//位平面数，必须为1	
    WORD biBitCount;//每像素位数	
    DWORD  biCompression; //压缩类型	
    DWORD  biSizeImage; //压缩图像大小字节数	
    LONG  biXPelsPerMeter; //水平分辨率	
    LONG  biYPelsPerMeter; //垂直分辨率	
    DWORD  biClrUsed; //位图实际用到的色彩数	
    DWORD  biClrImportant; //本位图中重要的色彩数
}BITMAPINFOHEADER; //位图信息头定义

typedef struct {	
    BYTE rgbBlue; //该颜色的蓝色分量	
    BYTE rgbGreen; //该颜色的绿色分量	
    BYTE rgbRed; //该颜色的红色分量	
    BYTE rgbReserved; //保留值
}RGBQUAD;//调色板定义

class BmpOps { 
public:
    int saveGrayBitmap(const char *fileName, BYTE *imgData, DWORD row, DWORD colume);
};

#endif