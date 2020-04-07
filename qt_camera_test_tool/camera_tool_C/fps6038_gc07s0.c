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
    unsigned long a : 8;
    unsigned long b : 8;
    unsigned long c : 4;
    unsigned long d : 4;
} * m_bit_p;

static unsigned char SENSOR_INIT_CONF[][4] = {
    {0xf0, 0x03, 0x30, 0xff}, {0xf0, 0x03, 0x30, 0x00}, {0xf0, 0x03, 0x30, 0xff}, {0xf0, 0x03, 0x30, 0x00},
    {0xf0, 0x03, 0x30, 0xff}, {0xf0, 0x03, 0x30, 0x00}, {0xf0, 0x03, 0x30, 0x20}, {0xf0, 0x03, 0x30, 0x00},
    {0xf0, 0x03, 0x04, 0xc6}, {0xf0, 0x03, 0x01, 0x10}, {0xf0, 0x03, 0x06, 0xcc}, {0xf0, 0x03, 0x05, 0x01},
    {0xf0, 0x03, 0x02, 0x18}, {0xf0, 0x02, 0xee, 0x30}, {0xf0, 0x02, 0x19, 0x0a},

    {0xf0, 0x01, 0x0e, 0xb0},                                                     // win_width
    {0xf0, 0x01, 0x10, 0xd8},                                                     // win_height
    {0xf0, 0x01, 0x0a, 0x04}, {0xf0, 0x01, 0x0c, 0x04}, {0xf0, 0x01, 0x12, 0xb0}, // out_width
    {0xf0, 0x01, 0x13, 0xd8},                                                     // out_height
    {0xf0, 0x01, 0x17, 0xd7},

    {0xf0, 0x02, 0x90, 0x01}, {0xf0, 0x02, 0x48, 0x40},

    {0xf0, 0x01, 0x60, 0x08}, {0xf0, 0x01, 0x61, 0x06}, {0xf0, 0x01, 0x62, 0x00}, {0xf0, 0x01, 0x63, 0x00},

    {0xf0, 0x00, 0x87, 0x52}, {0xf0, 0x01, 0x20, 0x01}, {0xf0, 0x01, 0x21, 0x02}, {0xf0, 0x01, 0x64, 0x08},

    {0xf0, 0x02, 0x24, 0x7c}, {0xf0, 0x02, 0x72, 0x17}, {0xf0, 0x02, 0x32, 0xc8}, {0xf0, 0x02, 0x61, 0x8c},
    {0xf0, 0x02, 0x42, 0x3e}, {0xf0, 0x02, 0x62, 0x1c}, {0xf0, 0x02, 0xcf, 0xc2}, {0xf0, 0x02, 0x34, 0x22},
    {0xf0, 0x02, 0xcd, 0x25},

    {0xf0, 0x02, 0x11, 0x20}, {0xf0, 0x02, 0x25, 0xa7}, {0xf0, 0x02, 0x21, 0x24}, {0xf0, 0x02, 0x29, 0x24},

    {0xf0, 0x02, 0x03, 0xf0}, {0xf0, 0x00, 0x89, 0x03}, {0xf0, 0x00, 0x60, 0x01},

    {0xf0, 0x01, 0x6f, 0x01}, {0xf0, 0x03, 0x06, 0xc8}, {0xf0, 0x02, 0x4e, 0x60}, {0xf0, 0x02, 0x2c, 0x2c},
    {0xf0, 0x02, 0xce, 0x60}, {0xf0, 0x02, 0x3b, 0x0e}, {0xf0, 0x02, 0x25, 0x27}, {0xf0, 0x02, 0x11, 0x00},
    {0xf0, 0x01, 0x42, 0x3f}, {0xf0, 0x01, 0x43, 0x02}, {0xf0, 0x01, 0x44, 0x3f}, {0xf0, 0x01, 0x45, 0x02},

    {0xf0, 0x02, 0x26, 0x30}, {0xf0, 0x04, 0x4c, 0x28}, {0xf0, 0x04, 0x4d, 0x28}, {0xf0, 0x04, 0x4e, 0x28},
    {0xf0, 0x04, 0x4f, 0x28}, {0xf0, 0x01, 0x71, 0x14},
};

static int spi_send_data(unsigned char *tx, unsigned char *rx, int len)
{
    cdfinger_spi_data data;
    int ret = -1;

    data.rx = rx;
    data.tx = tx;
    data.length = len;
    data.tx_length = len;
    ret = ioctl(private_fd, CDFINGER_SPI_WRITE_AND_READ, &data);
    if (ret != 0)
    {
        printf("spi send data is error!!! ret = %d\n", ret);
    }

    return ret;
}

static int sensor_sleep(void)
{
    unsigned char tx[3] = {0};
    unsigned char rx[3] = {0};
    if (sensor_power_flag != 0)
    {
        tx[0] = 0xfa;
        tx[1] = 0x04;
        tx[2] = 0x9b;
        if (spi_send_data(tx, rx, 3) < 0)
        {
            printf("sensor sleep failed!!!");
            return -1;
        }
        sensor_power_flag = 0;
    }

    return 0;
}

static int sensor_wakeup(void)
{
    if (sensor_power_flag != 1)
    {
        unsigned char tx[3] = {0};
        unsigned char rx[3] = {0};

        tx[0] = 0xfa;
        tx[1] = 0xc4;
        tx[2] = 0x9b;
        if (spi_send_data(tx, rx, 3) < 0)
        {
            printf("sensor wakeup failed!!!");
            return -1;
        }
        sensor_power_flag = 1;
    }
    return 0;
}

static bool sensor_verify_id(int fd)
{
    unsigned char tx[8] = {0};
    unsigned char rx[8] = {0};

    private_fd = fd;
    if (sensor_wakeup() < 0)
    {
        return false;
    }

    tx[0] = 0xf2;
    tx[1] = 0x03;
    tx[2] = 0xf0;
    if (spi_send_data(tx, rx, 8) < 0)
    {
        return false;
    }

    if (rx[4] != 0x07)
    {
        printf("fps6038 verify id failed=====%d!!!\n", rx[4]);
        return false;
    }

    if (sensor_sleep() < 0)
    {
        return false;
    }

    return true;
}

static int sensor_setImgWH(int width_base, int height_base, int width, int height)
{
    unsigned char tx[4] = {0};
    unsigned char rx[4] = {0};

    if (sensor_wakeup() < 0)
    {
        return -1;
    }

    tx[0] = 0xf0;
    tx[1] = 0x01;
    tx[2] = 0x15;
    tx[3] = width_base;
    if (spi_send_data(tx, rx, 4) < 0)
    {
        return -1;
    }

    tx[0] = 0xf0;
    tx[1] = 0x01;
    tx[2] = 0x16;
    tx[3] = height_base;
    if (spi_send_data(tx, rx, 4) < 0)
    {
        return -1;
    }

    tx[0] = 0xf0;
    tx[1] = 0x01;
    tx[2] = 0x12;
    tx[3] = width;
    if (spi_send_data(tx, rx, 4) < 0)
    {
        return -1;
    }

    tx[0] = 0xf0;
    tx[1] = 0x01;
    tx[2] = 0x13;
    tx[3] = height;
    if (spi_send_data(tx, rx, 4) < 0)
    {
        return -1;
    }

    tx[0] = 0xf0;
    tx[1] = 0x01;
    tx[2] = 0x17;
    tx[3] = height + height_base - 1;
    if (spi_send_data(tx, rx, 4) < 0)
    {
        return -1;
    }

    if (sensor_sleep() < 0)
    {
        return -1;
    }

    return 0;
}

static int sensor_init(int fd)
{
    int ret = 0, i = 0;
    int count = 0;
    unsigned char rx[4] = {0};

    private_fd = fd;
    ioctl(private_fd, CDFINGER_INIT_GPIO);

    if (sensor_wakeup() < 0)
    {
        return -1;
    }

    count = sizeof(SENSOR_INIT_CONF) / 4;
    for (i = 0; i < count; ++i)
    {
        ret = spi_send_data(SENSOR_INIT_CONF[i], rx, 4);
        if (ret < 0)
        {
            return -1;
        }
    }
    sensor_setImgWH(0, 30, SENSOR_WIDTH, SENSOR_HEIGHT);

    if (sensor_sleep() < 0)
    {
        return -1;
    }

    return 0;
}

static int sensor_setFrameNum(uint32_t count)
{

    return 0;
}

static int sensor_setExpoTime(int time)
{
    unsigned char tx[4] = {0};
    unsigned char rx[4] = {0};
    float row_time = 59.077;
    unsigned short exp = time * 1000 / 59.077;

    if (sensor_wakeup() < 0)
    {
        return -1;
    }

    tx[0] = 0xf0;
    tx[1] = 0x02;
    tx[2] = 0x02;
    tx[3] = exp >> 8;
    if (spi_send_data(tx, rx, 4) < 0)
    {
        return -1;
    }

    tx[0] = 0xf0;
    tx[1] = 0x02;
    tx[2] = 0x03;
    tx[3] = (unsigned char)exp;
    if (spi_send_data(tx, rx, 4) < 0)
    {
        return -1;
    }

    printf("==========exp=%d\n", exp);

    if (sensor_sleep() < 0)
    {
        return -1;
    }

    return 0;
}

static int sensor_setImgGain(unsigned char gain)
{
    unsigned char tx[5] = {0};
    unsigned char rx[5] = {0};

    if (sensor_wakeup() < 0)
    {
        return -1;
    }

    tx[0] = 0xf0;
    tx[1] = 0x02;
    tx[2] = 0xb3;
    if (gain <= 4)
    {
        tx[3] = gain;
    }
    else if (gain == 5)
    {
        tx[3] = 0x0c;
    }
    else if (gain >= 6)
    {
        tx[3] = 0x14;
    }
    if (spi_send_data(tx, rx, 4) < 0)
    {
        return -1;
    }

    if (sensor_sleep() < 0)
    {
        return -1;
    }

    return 0;
}

static int sensor_pre_image(void)
{
    unsigned char tx[4] = {0};
    unsigned char rx[4] = {0};

    if (sensor_wakeup() < 0)
    {
        return -1;
    }

    tx[0] = 0xf0;
    tx[1] = 0x01;
    tx[2] = 0x71;
    tx[3] = 0x14;
    if (spi_send_data(tx, rx, 4) < 0)
    {
        return -1;
    }

    tx[0] = 0xf0;
    tx[1] = 0x01;
    tx[2] = 0x2f;
    tx[3] = 0x01;
    if (spi_send_data(tx, rx, 4) < 0)
    {
        return -1;
    }

    return 0;
}

static void convert_img(unsigned short *dst, unsigned char *src, int width, int height)
{
    int len = width * height;
    int i = 0, j = 0;

    for (i = 0; i < len; i += 2)
    {
        m_bit_p = (struct m_bit *)(src + j * 3);
        dst[i] = ((m_bit_p->a << 4) | m_bit_p->d) & 0x0fff;
        dst[i + 1] = ((m_bit_p->b << 4) | m_bit_p->c) & 0x0fff;
        j++;
    }

    return;
}

static int sensor_get_img_buffer(unsigned short *img_buff, int width, int height)
{
    unsigned char *tx = NULL;
    unsigned char *rx = NULL;
    int spi_len = width * height * 12 / 8 + 4;

    tx = (unsigned char *)malloc(spi_len);
    if (tx == NULL)
    {
        printf("malloc failed,data.tx==FULL!!!\n");
        return -1;
    }
    rx = (unsigned char *)malloc(spi_len);
    if (rx == NULL)
    {
        printf("malloc failed,data.rx==FULL!!!\n");
        free(tx);
        return -1;
    }

    memset(tx, 0x00, spi_len);
    memset(rx, 0x00, spi_len);
    tx[0] = 0xf0;
    tx[1] = 0x01;
    tx[2] = 0x71;
    tx[3] = 0x04;
    if (spi_send_data(tx, rx, 4) < 0)
    {
        goto out;
    }

    tx[0] = 0xf4;
    tx[1] = ((spi_len - 4) & 0xff00) >> 8;
    tx[2] = ((spi_len - 4) & 0x00ff);
    tx[3] = 0x00;
    if (spi_send_data(tx, rx, spi_len) < 0)
    {
        goto out;
    }

    tx[0] = 0xf0;
    tx[1] = 0x01;
    tx[2] = 0x6f;
    tx[3] = 0x00;
    if (spi_send_data(tx, rx, 4) < 0)
    {
        goto out;
    }
    convert_img(img_buff, rx + 4, width, height);
    if (sensor_sleep() < 0)
    {
        return -1;
    }

    free(rx);
    free(tx);
    return 0;

out:
    free(rx);
    free(tx);
    return -1;
}

static int sensor_config_process(char *img_path, int fd)
{

    return 0;
}

static int sensor_setBinning(int binning_mode)
{

    return 0;
}

void fps6038_gc07s0_init(cdfinger_fops *sensor_fops)
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