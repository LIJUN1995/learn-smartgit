#include <stdio.h>
#include <stdlib.h>

int fp_SaveGrayBitmap(const char *FilePath, unsigned char *pData, int row, int colume)
{
	/*Check parameter*/
	FILE *fd;
	unsigned char grayBitmapHeader[1078] = {0};
	unsigned char pad[4] = {0};
	int colume_t = 0;
	int row_t = 0;
	int i = 0;
	if (NULL == pData)
	{
		return -1;
	}
	fd = fopen(FilePath, "wb+");
	if (fd == NULL)
	{
		printf("\nthe file open error\n");
		return -1;
	}
	/*Gray bitmap file header*/
	{
		colume_t = (colume + 3) & 0xFC;
		row_t = (row + 3) & 0xFC;
		grayBitmapHeader[0] = 0x42;
		grayBitmapHeader[1] = 0x4d;
		grayBitmapHeader[2] = 0x36;
		grayBitmapHeader[3] = 0x28;
		grayBitmapHeader[4] = 0x00;
		grayBitmapHeader[5] = 0x00;
		grayBitmapHeader[6] = 0x00;
		grayBitmapHeader[7] = 0x00;
		grayBitmapHeader[8] = 0x00;
		grayBitmapHeader[9] = 0x00;
		grayBitmapHeader[10] = 0x36;
		grayBitmapHeader[11] = 0x04;
		grayBitmapHeader[12] = 0x00;
		grayBitmapHeader[13] = 0x00;
		grayBitmapHeader[14] = 0x28;
		grayBitmapHeader[15] = 0x00;
		grayBitmapHeader[16] = 0x00;
		grayBitmapHeader[17] = 0x00;

		grayBitmapHeader[18] = (colume_t & 0xFF);
		grayBitmapHeader[19] = (colume_t >> 8 & 0xFF);
		grayBitmapHeader[20] = (colume_t >> 16 & 0xFF);
		grayBitmapHeader[21] = (colume_t >> 24 & 0xFF);

		grayBitmapHeader[22] = (row_t & 0xFF);
		grayBitmapHeader[23] = (row_t >> 8 & 0xFF);
		grayBitmapHeader[24] = (row_t >> 16 & 0xFF);
		grayBitmapHeader[25] = (row_t >> 24 & 0xFF);

		grayBitmapHeader[26] = 0x01;
		grayBitmapHeader[27] = 0x00;

		grayBitmapHeader[28] = 0x08;
		grayBitmapHeader[29] = 0x00;

		for (i = 0; i < 256; i++)
		{
		grayBitmapHeader[54 + i * 4] = i;
		grayBitmapHeader[54 + i * 4 + 1] = i;
		grayBitmapHeader[54 + i * 4 + 2] = i;
		}
		int ret = fwrite((char *)grayBitmapHeader,1 , 1078, fd);
		/*Be careful , bitmap save turn is buttom to top*/
		for (i = 0; i < row; i++)
		{
		ret = fwrite((char *)(pData + (row - i - 1) * colume), 1, colume, fd);
		
		if (colume_t > colume)
			ret = fwrite((char *)pad, 1, colume_t - colume, fd); 
		
		}
		fseek(fd,0,SEEK_SET);
		fseek(fd,0,SEEK_END);
		long longBytes=ftell(fd);// longBytes就是文件的长度
		printf("longBytes=%ld\n",longBytes);
		fclose(fd);
	}
	return 0;
	}

int fp_SaveGrayBmpFile(const char *FilePath, unsigned char *pData,
			int row, int colume, int bitWide)
{
	int i;
	int data_len;
	data_len = row * colume;

	if (bitWide == 8)
	{
		return fp_SaveGrayBitmap(FilePath, pData, row, colume);
	}

	if (bitWide == 12)
	{
		unsigned char tmp_data[128 * 128];
		if (data_len > sizeof(tmp_data))
		data_len = sizeof(tmp_data);
		unsigned short *iData = (unsigned short *)pData;
		for (i = 0; i < data_len; i++)
		{
		tmp_data[i] = (((pData[2 * i] >> 4) & 0x0F) | ((pData[2 * i + 1] << 4) & 0xF0)) & 0xFF;
		//tmp_data[i] = iData[i]/16;
		}
		return fp_SaveGrayBitmap(FilePath, tmp_data, row, colume);
	}
	return -1;
}

int fp_SaveBinFile(const char *FilePath, unsigned char *pData,int row, int colume)
{
	FILE *fb = NULL;
	fb = fopen(FilePath,"wb+");
	if(fb == NULL){
		printf("open failed\n");
		return -1;
	}
	fwrite(pData,row,colume,fb);
	fclose(fb);

	return 0;
}

int get_image_data(char *img_path,unsigned char *buf)
{
    int length = 0,ret = 0;

    FILE *fp = fopen(img_path,"rb+");
    if(fp == NULL){
        printf("fopen file failed!!!\n");
        return -1;
    }
    fseek(fp, 0L, SEEK_END);
    length = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    ret = fread(buf, sizeof(unsigned char), length, fp);
    if (ret != length){
        printf("fread failed!!!\n");
        return -1;
    }

    fclose(fp);
    return length;
}

int main(int argc, char *argv[])
{
    int ret = 0, length = 0;
    char file_name[64] = {0};

    if(argc != 2){
        printf("please input two parameter\n");
        return -1;
    }

    unsigned char *buffer = (unsigned char *)malloc(1024*400);
    if(buffer == NULL){
        printf("malloc failed!!!\n");
        goto out;
    }

    length = get_image_data(argv[1],buffer);
    if(length < 0){
        printf("get_image_data failed\n");
        return -1;
    }

    sprintf(file_name,"%s%s",argv[1],"_new.bin");

    FILE *fp = fopen(file_name,"ab+");
    if(fp == NULL){
        printf("fopen bg_new.bin failed!!!\n");
        return -1;
    }

    ret = fwrite(buffer,length,1,fp);
    if(ret != 1){
        printf("fwrite failed!!!\n");
        goto out;
    }
    ret = fwrite(buffer,length,1,fp);
    if(ret != 1){
        printf("fwrite failed!!!\n");
        goto out;
    }
    fclose(fp);

    length = get_image_data(file_name,buffer);
    if(length < 0){
        printf("get_image_data failed\n");
        return -1;
    }

    sprintf(file_name,"%s%s",argv[1],"_new.bmp");
    ret = fp_SaveGrayBmpFile(file_name, buffer, 144, 96, 12);
 
out:
    free(buffer);
    
    return 0;
}