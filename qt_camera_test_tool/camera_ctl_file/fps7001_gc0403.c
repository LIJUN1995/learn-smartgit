#include "sensor_ctl.h"
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

static int private_fd = -1;
static int sensor_power_flag = -1;
static int _current_fusion_frame_number = 4;

static struct m_bit
{
    unsigned long long a1 : 10;
    unsigned long long b1 : 10;
    unsigned long long c1 : 10;
    unsigned long long d1 : 10;
} * m_bit_p;

static unsigned char SENSOR_INIT_CONF[][2] = {
    {0xfe, 0x80}, {0xfe, 0x80}, {0xfe, 0x80}, {0xf2, 0x00}, {0xf6, 0x00}, {0xfc, 0xc6}, {0xf7, 0x19}, {0xf8, 0x01},
    {0xf9, 0x3e}, {0xfe, 0x03}, {0x06, 0x80}, {0x06, 0x00}, {0xfe, 0x00}, {0xf9, 0x2e}, {0xfe, 0x00}, {0xfa, 0x00},
    {0xfe, 0x00}, {0x03, 0x0a}, {0x04, 0x55}, {0x05, 0x00}, {0x06, 0xbb}, {0x07, 0x00}, {0x08, 0x46}, {0x0c, 0x04},
    {0x0d, 0x02}, {0x0e, 0x48}, {0x0f, 0x03}, {0x10, 0x08}, {0x11, 0x23}, {0x12, 0x10}, {0x13, 0x11}, {0x14, 0x01},
    {0x15, 0x00}, {0x16, 0xc0}, {0x17, 0x14}, {0x18, 0x02}, {0x19, 0x38}, {0x1a, 0x11}, {0x1b, 0x06}, {0x1c, 0x04},
    {0x1d, 0x00}, {0x1e, 0xfc}, {0x1f, 0x09}, {0x20, 0xb5}, {0x21, 0x3f}, {0x22, 0xe6}, {0x23, 0x32}, {0x24, 0x2f},
    {0x27, 0x00}, {0x28, 0x00}, {0x2a, 0x00}, {0x2b, 0x03}, {0x2c, 0x00}, {0x2d, 0x01}, {0x2e, 0xf0}, {0x2f, 0x01},
    {0x25, 0xc0}, {0x3d, 0xe0}, {0x3e, 0x45}, {0x3f, 0x1f}, {0xc2, 0x17}, {0x30, 0x22}, {0x31, 0x23}, {0x32, 0x02},
    {0x33, 0x03}, {0x34, 0x04}, {0x35, 0x05}, {0x36, 0x06}, {0x37, 0x07}, {0x38, 0x0f}, {0x39, 0x17}, {0x3a, 0x1f},
    {0xb6, 0x04}, {0xfe, 0x00}, {0x8a, 0x00}, {0x8c, 0x07}, {0x8e, 0x02}, {0x90, 0x01}, {0x94, 0x02}, {0x95, 0x02},
    {0x96, 0x40}, {0x97, 0x03}, {0x98, 0x00}, {0xfe, 0x00}, {0x18, 0x02}, {0x40, 0x22}, {0x41, 0x01}, {0x5e, 0x00},
    {0x66, 0x20}, {0xfe, 0x03}, {0x01, 0x83}, {0x02, 0x11}, {0x03, 0x96}, {0x04, 0x01}, {0x05, 0x00}, {0x06, 0xa4},
    {0x10, 0x90}, {0x11, 0x2b}, {0x12, 0xc0}, {0x13, 0x03}, {0x15, 0x02}, {0x21, 0x10}, {0x22, 0x03}, {0x23, 0x20},
    {0x24, 0x02}, {0x25, 0x10}, {0x26, 0x05}, {0x21, 0x10}, {0x29, 0x03}, {0x2a, 0x0a}, {0x2b, 0x04}, {0xfe, 0x00},
    {0xb0, 0x50}, {0xb6, 0x01},

    /*出固定数据192*/
    // {0x8b, 0xb2}, {0x8d, 0x38},

    /*出测试图*/
    // {0x8b, 0xb2}, {0x8d, 0x03},
};

static int spi_send_data(unsigned char *tx, unsigned char *rx, int len)
{
    int ret = -1;
    cdfinger_spi_data data;

    data.tx = tx;
    data.rx = rx;
    data.length = len;
    data.tx_length = len;

    ret = ioctl(private_fd, CDFINGER_SPI_WRITE_AND_READ, &data);
    if (ret != 0)
    {
        printf("spi send data is error!!!\n");
    }

    return ret;
}

static int i2c_write_data(unsigned short reg_address, unsigned char reg_data)
{
    unsigned char tx[6] = {0};
    unsigned char rx[6] = {0};
    int count = 0;

    tx[0] = 0x82;
    do
    {
        if (spi_send_data(tx, rx, 4) < 0)
        {
            return -1;
        }
        if (count > 100)
        {
            printf("I2C status is busy,status=0x%02x!!!\n", rx[3]);
            return -1;
        }
        ++count;
    } while ((rx[3] & 0x80) == 0x80);
    count = 0;

    /*set slaver device id*/
    tx[0] = 0x81;
    tx[1] = 0x78;
    tx[2] = 0x66;
    if (spi_send_data(tx, rx, 3) < 0)
    {
        return -1;
    }

    /*set reg_address and reg_data*/
    tx[0] = 0x85;
    tx[1] = 0x00;
    tx[2] = 0x66;
    tx[3] = (unsigned char)(reg_address);
    tx[4] = reg_data;
    if (spi_send_data(tx, rx, 5) < 0)
    {
        return -1;
    }

    /*set the number of bytes to send*/
    tx[0] = 0x83;
    tx[1] = 0x02;
    tx[2] = 0x66;
    if (spi_send_data(tx, rx, 3) < 0)
    {
        return -1;
    }

    /*power on*/
    tx[0] = 0x87;
    tx[1] = 0x80;
    tx[2] = 0x66;
    if (spi_send_data(tx, rx, 3) < 0)
    {
        return -1;
    }

    tx[0] = 0x87;
    tx[1] = 0x00;
    tx[2] = 0x66;
    if (spi_send_data(tx, rx, 3) < 0)
    {
        return -1;
    }

    do
    {
        memset(tx, 0x66, 6);
        memset(rx, 0x00, 6);
        tx[0] = 0x82;
        if (spi_send_data(tx, rx, 4) < 0)
        {
            return -1;
        }
        if (count > 3)
        {
            printf("I2C status is not transfer done, status=0x%02x!!!\n", rx[3]);
            return -1;
        }
        ++count;
    } while (rx[3] != 0x08);

    return 0;
}

static int i2c_read_data(unsigned short reg_address, unsigned char *reg_data)
{
    unsigned char tx[6] = {0};
    unsigned char rx[6] = {0};
    int count = 0;

    do
    {
        tx[0] = 0x82;
        if (spi_send_data(tx, rx, 4) < 0)
        {
            return -1;
        }
        if (count > 10)
        {
            printf("I2C status is busy,status=0x%02x!!!\n", rx[3]);
            return -1;
        }
        ++count;
    } while ((rx[3] & 0x80) == 0x80);
    count = 0;

    /*set slaver device id*/
    tx[0] = 0x81;
    tx[1] = 0x79;
    tx[2] = 0x66; // 0x00
    if (spi_send_data(tx, rx, 3) < 0)
    {
        return -1;
    }

    /*set reg_address and reg_data*/
    tx[0] = 0x85;
    tx[1] = 0x00;
    tx[2] = 0x66;
    tx[3] = (unsigned char)(reg_address);
    if (spi_send_data(tx, rx, 4) < 0)
    {
        return -1;
    }

    /*set the number of bytes to send*/
    tx[0] = 0x83;
    tx[1] = 0x00;
    tx[2] = 0x66;
    if (spi_send_data(tx, rx, 3) < 0)
    {
        return -1;
    }

    /*power on*/
    tx[0] = 0x87;
    tx[1] = 0x90; // 1001 1000
    tx[2] = 0x66;
    if (spi_send_data(tx, rx, 3) < 0)
    {
        return -1;
    }

    tx[0] = 0x87;
    tx[1] = 0x10; //
    tx[2] = 0x66;
    if (spi_send_data(tx, rx, 3) < 0)
    {
        return -1;
    }

    do
    {
        memset(tx, 0x66, 6);
        memset(tx, 0x00, 6);
        tx[0] = 0x82;
        if (spi_send_data(tx, rx, 4) < 0)
        {
            return -1;
        }
        if (count > 10)
        {
            printf("I2C status is not transfer done and Lost arbitration, status=0x%02x!!!\n", rx[3]);
            return -1;
        }
        ++count;
    } while (rx[3] != 0x0c);

    tx[0] = 0x86;
    tx[1] = 0x00;
    tx[2] = 0x66;
    tx[3] = 0x66;
    if (spi_send_data(tx, rx, 4) < 0)
    {
        return -1;
    }

    *reg_data = rx[3];

    return 0;
}

static int checkUpdate(void)
{
    uint8_t tx[8] = {0};
    uint8_t rx[8] = {0};

    memset(tx, 0x66, 8);
    memset(rx, 0x66, 8);
    tx[0] = 0x8e;
    tx[1] = 0x66;
    tx[2] = 0x66;
    tx[3] = 0x66;
    spi_send_data(tx, rx, 8);

    if (rx[3] == 0xa5)
    {
        printf("===============the module is single=================");
    }
    else if (rx[3] == 0xa6)
    {
        printf("===============the module is double=================");
        return -1;
    }
    else
    {
        printf("===============download program failed=================");
        printf("0x%02x  0x%02x  0x%02x  0x%02x  0x%02x  0x%02x  0x%02x  0x%02x", rx[0], rx[1], rx[2], rx[3], rx[4], rx[5],
             rx[6], rx[7]);

        return -1;
    }

    return 0;
}

static bool sensor_verify_id(int fd)
{
    int ret = -1;
    unsigned char sensor_id = 0;

    private_fd = fd;
    ioctl(private_fd, CDFINGER_INIT_GPIO);

    ret = checkUpdate();
    if (ret < 0)
    {
        return false;
    }

    /*read sensor ID*/
    ret = i2c_read_data(0xf0, &sensor_id);
    if ((ret < 0) || (sensor_id != 0x04))
    {
        printf("fps7001_gc0403 read id failed! ret = %d, id = 0x%02x\n", ret, sensor_id);
        return false;
    }

    return true;
}
void send_reg_cfg(uint8_t reg, uint8_t value)
{
    uint8_t tx[8] = {0};
    uint8_t rx[8] = {0};

    memset(tx, 0x66, 8);
    memset(rx, 0x66, 8);
    tx[0] = reg;
    tx[1] = value;
    tx[2] = 0x66;
    tx[3] = 0x66;
    spi_send_data(tx, rx, 8);
}

static int sensor_init(int fd)
{
    /*read id*/
    unsigned char tx[10] = {0};
    unsigned char rx[10] = {0};
    int reg_count = 0;
    int ret = 0;
    int i = 0;

    //////////////////////   reset cpld   //////////////////////
    tx[0] = 0xf0;
    tx[1] = 0x66;
    tx[2] = 0x66;
    tx[3] = 0x66;
    if (spi_send_data(tx, rx, 4) < 0)
    {
        return -1;
    }

    tx[0] = 0x8d;
    tx[1] = 0x00;
    tx[2] = 0x66;
    tx[3] = 0x66;
    if (spi_send_data(tx, rx, 4) < 0)
    {
        return -1;
    }

    /*set camera*/
    //////////////////////   IRQ enable   //////////////////////
    tx[0] = 0x88;
    tx[1] = 0x66;
    tx[2] = 0x66;
    tx[3] = 0x66;
    if (spi_send_data(tx, rx, 4) < 0)
    {
        return -1;
    }

    //////////////////////   IRQ clear   //////////////////////
    tx[0] = 0x89;
    tx[1] = 0x66;
    tx[2] = 0x66;
    if (spi_send_data(tx, rx, 3) < 0)
    {
        return -1;
    }

    reg_count = sizeof(SENSOR_INIT_CONF) / 2;
    printf("the reg_count = %d\n", reg_count);
    for (i = 0; i < reg_count; i++)
    {
        if (i2c_write_data(SENSOR_INIT_CONF[i][0], SENSOR_INIT_CONF[i][1]) == -1)
        {
            printf("i2c write data error\n");
            return -1;
        }
    }

    tx[0] = 0x88;
    tx[1] = 0x66;
    tx[2] = 0x66;
    tx[3] = 0x66;
    if (spi_send_data(tx, rx, 4) < 0)
    {
        return -1;
    }
    // sensor_setFrameNum(4);
    send_reg_cfg(0x91, 1);
    send_reg_cfg(0x92, 1);

    return 0;
}

static int sensor_setFrameNum(uint32_t count)
{
    if (count != _current_fusion_frame_number)
    {
        _current_fusion_frame_number = count;
    }
    unsigned char tx[4] = {0};
    unsigned char rx[4] = {0};

    tx[0] = 0x90;
    tx[1] = count;
    tx[2] = 0x66;
    tx[3] = 0x66;
    if (spi_send_data(tx, rx, 4) < 0)
    {
        return -1;
    }

    return 0;
}

static int sensor_setImgWH(int width_base, int height_base, int width, int height)
{
    /*
       unsigned char temp_v1 = 0;
       unsigned char temp_v2 = 0;

       temp_v2 = (unsigned char)height_base;
       temp_v1 = (unsigned char)(height_base >> 8);
       if (i2c_write_data(0x91, temp_v1) == -1)
       {
           printf("i2c write data error\n");
           return -1;
       }
       if (i2c_write_data(0x92, temp_v2) == -1)
       {
           printf("i2c write data error\n");
           return -1;
       }

       temp_v2 = (unsigned char)width_base;
       temp_v1 = (unsigned char)(width_base >> 8);
       if (i2c_write_data(0x93, temp_v1) == -1)
       {
           printf("i2c write data error\n");
           return -1;
       }
       if (i2c_write_data(0x94, temp_v2) == -1)
       {
           printf("i2c write data error\n");
           return -1;
       }

       temp_v2 = (unsigned char)height;
       temp_v1 = (unsigned char)(height >> 8);
       if (i2c_write_data(0x95, temp_v1) == -1)
       {
           printf("i2c write data error\n");
           return -1;
       }
       if (i2c_write_data(0x96, temp_v2) == -1)
       {
           printf("i2c write data error\n");
           return -1;
       }

       temp_v2 = (unsigned char)width;
       temp_v1 = (unsigned char)(width >> 8);
       if (i2c_write_data(0x97, temp_v1) == -1)
       {
           printf("i2c write data error\n");
           return -1;
       }
       if (i2c_write_data(0x98, temp_v2) == -1)
       {
           printf("i2c write data error\n");
           return -1;
       }
   */
    return 0;
}

static int sensor_setExpoTime(int time)
{
    int frame_time = 0;
    int VB = 0X46;
    int HB = 0Xbb;
    unsigned char vb1 = 0, vb2 = 0;
    unsigned char hb1 = 0, hb2 = 0;
    float row_time = 0;
    int exp = 0;

    unsigned char reg_03 = 0, reg_04 = 0;

    row_time = (406 + HB) / 10000.00;
    exp = time / row_time;

    printf("exp == %d\n", exp);

    if (exp <= 774 + VB)
    {
        frame_time = 774 + VB;
    }
    else
    {
        frame_time = exp * row_time;
    }

    reg_03 = exp >> 8;
    reg_04 = exp;
    printf("reg_03=0x%02x, reg_04=0x%02x\n", reg_03, reg_04);

    if (i2c_write_data(0xfe, 0x00) != 0)
    {
        printf("i2c write 0x03 error\n");
        return -1;
    }

    if (i2c_write_data(0x03, reg_03) != 0)
    {
        printf("i2c write 0x03 error\n");
        return -1;
    }
    if (i2c_write_data(0x04, reg_04) != 0)
    {
        printf("i2c write 0x04 error\n");
        return -1;
    }

    i2c_read_data(0x03, &reg_03);
    i2c_read_data(0x04, &reg_04);
    printf("reg_03=0x%02x, reg_04=0x%02x\n", reg_03, reg_04);

    return 0;
}

static int sensor_setImgGain(unsigned char gain)
{

    i2c_write_data(0xfe, 0x00);
    if (i2c_write_data(0xb6, gain) != 0)
    {
        printf("i2c write data error\n");
        return -1;
    }

    // usleep(100);
    // unsigned char gain_1 = 0;
    // i2c_read_data(0xb6, &gain_1);
    // printf("gain=0x%02x\n", gain_1);

    return 0;
}

static int sensor_pre_image(void)
{
    unsigned char tx[6] = {0};
    unsigned char rx[6] = {0};

    memset(tx, 0x66, 4);
    memset(rx, 0x66, 4);

    tx[0] = 0x8d;
    tx[1] = 0x40;
    if (spi_send_data(tx, rx, 4) < 0)
    {
        return -1;
    }

    return 0;
}

int get_image_data(char *img_path, unsigned char *buf)
{
    int length = 0, ret = 0;

    FILE *fp = fopen(img_path, "rb+");
    if (fp == NULL)
    {
        printf("fopen file failed!!!\n");
        return -1;
    }
    fseek(fp, 0L, SEEK_END);
    length = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    ret = fread(buf, sizeof(unsigned char), length, fp);
    if (ret != length)
    {
        printf("fread failed!!!\n");
        return -1;
    }

    fclose(fp);
    return length;
}
static int sensor_get_img_buffer(unsigned short *img_buff, int width, int height)
{
    int ret = 0;

    ret = get_image_data("/data/img_new.bin", (unsigned char *)img_buff);
    printf("ret=%d!!!!!!!!!!!!!!!!\n", ret);

    return 0;

    /*   unsigned char *tx;
       unsigned char *rx;
       int spi_len = width * height * 10 / 8 + 3;
       int ret = 0, i = 0, j = 0;

       tx = (unsigned char *)malloc(spi_len);
       if (tx == NULL)
       {
           printf("malloc failed,tx==FULL!!!\n");
           return -1;
       }
       rx = (unsigned char *)malloc(spi_len);
       if (rx == NULL)
       {
           printf("malloc failed,rx==FULL!!!\n");
           free(tx);
           return -1;
       }

       memset(tx, 0x66, spi_len);
       memset(rx, 0x66, spi_len);

       tx[0] = 0x8a;

       if (spi_send_data(tx, rx, spi_len) < 0)
       {
           goto out;
       }

       for (i = 0; i < (spi_len - 3) / 5; ++i)
       {
           m_bit_p = (struct m_bit *)(rx + 3 + i * 5);
           img_buff[j] = m_bit_p->a1 << 2;
           img_buff[j + 1] = m_bit_p->b1 << 2;
           img_buff[j + 2] = m_bit_p->c1 << 2;
           img_buff[j + 3] = m_bit_p->d1 << 2;
           j += 4;
       }

       free(rx);
       free(tx);
       return 0;
   out:
       free(rx);
       free(tx);
       return -1;
   */
}

static void send_clk(int clk_count)
{
    for (int i = 0; i < clk_count; ++i)
    {
        ioctl(private_fd, CDFINGER_CONTROL_CLK, 0);
        usleep(1000);
        ioctl(private_fd, CDFINGER_CONTROL_CLK, 1);
        usleep(1000);
    }
}

static int sensor_config_process(char *img_path, int fd)
{
    int i = 0, length = 0, ret = -1;
    unsigned char *tx = NULL;
    unsigned char *rx = NULL;
    private_fd = fd;

    FILE *fp = fopen("/data/system/cdfinger/cpld_single.bin", "rb+");
    if (fp == NULL)
    {
        printf("open file error %s\n", img_path);
        goto out;
    }
    fseek(fp, 0L, SEEK_END);
    length = ftell(fp);
    printf("bin file length = %d ,path=%s\n", length, img_path);
    fseek(fp, 0L, SEEK_SET);
    tx = (unsigned char *)malloc(length);
    if (tx == NULL)
    {
        printf("malloc failed,data.tx==FULL!!!\n");
        goto out;
    }
    rx = (unsigned char *)malloc(length);
    if (rx == NULL)
    {
        printf("malloc failed,data.rx==FULL!!!\n");
        goto out;
    }

    ioctl(private_fd, CDFINGER_HW_RESET, 0);
    ioctl(private_fd, CDFINGER_CONTROL_CS, 0);
    ioctl(private_fd, CDFINGER_CONTROL_CLK, 1);
    usleep(1000);
    ioctl(private_fd, CDFINGER_HW_RESET, 1);
    usleep(1000);
    ioctl(private_fd, CDFINGER_CONTROL_CS, 1);

    send_clk(8);

    ioctl(private_fd, CDFINGER_INIT_GPIO);
    ioctl(private_fd, CDFINGER_SPI_MODE, 3);

    ret = fread(tx, sizeof(uint8_t), length, fp);
    if (ret != length)
    {
        printf("fread error!!!!!!!!!!!!!!!!!!!!!!!\n");
        ret = -1;
        goto out;
    }

    if (spi_send_data(tx, rx, length) < 0)
    {
        ret = -1;
        goto out;
    }

    send_clk(200);

    ioctl(private_fd, CDFINGER_INIT_GPIO);
    ioctl(private_fd, CDFINGER_SPI_MODE, 0);

    ret = 0;
out:
    if (tx != NULL)
        free(tx);
    if (rx != NULL)
        free(rx);
    if (fp != NULL)
        fclose(fp);
    return ret;
}

static int sensor_get_gain(void)
{
    int ret = -1;
    unsigned char rd_gain = 0;

    ret = i2c_read_data(0xb6, &rd_gain);
    if (ret < 0)
    {
        printf("spi read sensor gain error!!!\n");
        return -1;
    }

    printf("lj gain_b6=0x%02x\n", rd_gain);

    return rd_gain;
}

static int sensor_setBinning(int binning_mode)
{

    return 0;
}

static int sensor_sleep(void)
{

    return 0;
}

static int sensor_wakeup(void)
{

    return 0;
}

void fps7001_gc0403_init(cdfinger_fops *sensor_fops)
{
    sensor_fops->sensor_verify_id = sensor_verify_id;
    sensor_fops->sensor_init = sensor_init;
    sensor_fops->sensor_setImgWH = sensor_setImgWH;
    sensor_fops->sensor_pre_image = sensor_pre_image;
    sensor_fops->sensor_setBinning = sensor_setBinning;
    sensor_fops->sensor_setImgGain = sensor_setImgGain;
    sensor_fops->sensor_setExpoTime = sensor_setExpoTime;
    sensor_fops->sensor_setFrameNum = sensor_setFrameNum;
    sensor_fops->sensor_get_img_buffer = sensor_get_img_buffer;
    sensor_fops->sensor_config_process = sensor_config_process;
    sensor_fops->sensor_sleep = sensor_sleep;
    sensor_fops->sensor_wakeup = sensor_wakeup;

    return;
}
