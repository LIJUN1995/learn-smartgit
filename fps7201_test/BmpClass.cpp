#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>
// #include <fcntl.h>
// #include <signal.h>
// #include <sys/time.h>
// #include <semaphore.h>

#include "BmpClass.h"

int BmpOps::saveGrayBitmap(const char *fileName, BYTE *imgData, DWORD row, DWORD colume)
{
    const DWORD height = row;
    const DWORD width = colume;
    uint32_t palette[256] = {0};
    uint8_t *p = NULL;

    uint8_t m[1078] = {0};

    for (size_t i = 0; i < 256; i++)
    {
        palette[i] = (i<<16) | (i<<8) | i;
        // std::cout<<std::hex<<palette[i]<<std::dec<<std::endl;
    }
    

    BITMAPFILEHEADER fileHeader = {0};
    BITMAPINFOHEADER bitmapHeader = {0};

    fileHeader.bfType = 0x4d42;
    fileHeader.bfSize = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+sizeof(palette)+height*width;
    fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+sizeof(palette);

    // std::cout<<sizeof(BITMAPFILEHEADER)<<std::endl;
    // std::cout<<sizeof(BITMAPINFOHEADER)<<std::endl;
    // std::cout<<sizeof(palette)<<std::endl;

    bitmapHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmapHeader.biHeight = height;
    bitmapHeader.biWidth = width;
    bitmapHeader.biPlanes = 1;
    bitmapHeader.biBitCount = 8;
    bitmapHeader.biCompression = 0; //BI_RGB
    bitmapHeader.biClrUsed = 256;
    bitmapHeader.biClrImportant = 256;


    FILE *fp = fopen(fileName, "w+");
    if(fp == NULL)
    {
        std::cerr<<"fopen file "<<fileName<<" failed"<<std::endl;
        return -1;
    }

    fwrite(&fileHeader, 1, sizeof(BITMAPFILEHEADER), fp);
    fwrite(&bitmapHeader, 1, sizeof(BITMAPINFOHEADER), fp);
    fwrite(palette, 1, sizeof(palette), fp);

    for (size_t i = 0; i < height; i++)
    {
        fwrite(imgData+(height-i-1)*width, 1, width, fp);
    }

    fclose(fp);

    return 0;
}