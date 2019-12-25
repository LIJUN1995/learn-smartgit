/*************************************************************************
        > File Name: fps6037_isf1001.c
        > Author:fpw
        > Mail:
        > Created Time: 2019年09月15日 星期日 21时01分50秒
 ************************************************************************/
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "cfp_mem.h"
#include "sensor_ctl.h"

#define HEAD_SIZE 1
#define LINE_NUM_SIZE 1
#define HIST_SIZE 33
#define HIST_M_SIZE 17
#define ISF_CONFIG_REG_LEN 3
#define IMAGE_CMD_SIZE 6
#define SENSOR_DOL 1
#define SENSOR_FULL_H 176
#define SENSOR_FULL_W 176
#define BYTE_SIZE 2

static int private_fd = -1;
static int sensor_power_flag = -1;
static unsigned char *g_buffer = NULL;
static unsigned char *g_image = NULL;

static unsigned char init_config[][ISF_CONFIG_REG_LEN] = {
    {0x81, 0x4b, 0x1}, {0x81, 0x46, 0x1}, {0x81, 0x7c, 0x1}, {0x81, 0x2e, 0x1}, {0x81, 0x2f, 0x1},
    {0x81, 0x51, 0x1}, {0x81, 0x59, 0x1}, {0x81, 0x5b, 0x6}, {0x81, 0x5c, 0x1}, {0x81, 0x9b, 0x0},
    {0x81, 0x84, 0x0}, {0x83, 0x41, 0x0}, {0x82, 0x98, 0x0}, // FULL
    {0x81, 0xab, 0x0},                                       // SHUT
    {0x81, 0x6c, 30},  {0x81, 0x6d, 0x0}, {0x81, 0x6e, 205}, {0x81, 0x6f, 0x0}, {0x81, 0x70, 30},
    {0x81, 0x71, 0x0}, {0x81, 0x72, 205}, {0x81, 0x73, 0x0}, // 12bit
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
    unsigned char sleep_config[][ISF_CONFIG_REG_LEN] = {
        {0x81, 0x2e, 0x00}, {0x81, 0x2f, 0x00}, {0x81, 0x59, 0x00},
        {0x81, 0x5c, 0x00}, {0x81, 0x46, 0x00}, {0x81, 0x4b, 0x00},
    };
    unsigned char rx[3] = {0};
    if (sensor_power_flag != 0)
    {
        for (size_t i = 0; i < 6; i++)
        {
            if (spi_send_data(sleep_config[i], rx, 3) < 0)
            {
                printf("sensor_sleep fail");
                return -1;
            }
        }
        sensor_power_flag = 0;
    }

    printf("sensor_sleep ok");
    return 0;
}

static int sensor_wakeup(void)
{
    unsigned char wakeup_config[][ISF_CONFIG_REG_LEN] = {
        {0x81, 0x4b, 0x01}, {0x81, 0x2e, 0x01}, {0x81, 0x2f, 0x01},
        {0x81, 0x59, 0x01}, {0x81, 0x5c, 0x01}, {0x81, 0x46, 0x01},
    };
    unsigned char rx[3] = {0};
    if (sensor_power_flag != 1)
    {
        for (size_t i = 0; i < 6; i++)
        {
            if (spi_send_data(wakeup_config[i], rx, 3) < 0)
            {
                return -1;
            }
        }
        sensor_power_flag = 1;
    }
    printf("sensor_wakeup ok");
    return 0;
}

static bool sensor_verify_id(int fd)
{
    unsigned char tx[3] = {0};
    unsigned char rx[3] = {0};

    tx[0] = 0x01;
    tx[1] = 0x31;
    private_fd = fd;
    if (spi_send_data(tx, rx, 3) < 0)
    {
        return false;
    }
    printf("ISF1001 verify_id = 0x%x\n", rx[2]);
    if (rx[2] != 0x88)
    {
        printf("fps6037 verify id failed=====%d!!!\n", rx[2]);
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
    unsigned char temp_v1 = 0;
    unsigned char temp_v2 = 0;
    unsigned int h = 0;
    unsigned int w = 0;
    unsigned char tx[3] = {0};
    unsigned char rx[3] = {0};

    if (sensor_wakeup() < 0)
    {
        return -1;
    }

    temp_v2 = (unsigned char)width_base;
    temp_v1 = (unsigned char)((width_base >> 8) & 0x01);
    tx[0] = 0x81;
    tx[1] = 0x6c;
    tx[2] = temp_v2;
    if (spi_send_data(tx, rx, 3) < 0)
    {
        return -1;
    }
    tx[0] = 0x81;
    tx[1] = 0x6d;
    tx[2] = temp_v1;
    if (spi_send_data(tx, rx, 3) < 0)
    {
        return -1;
    }

    w = width_base + width - 1;
    temp_v2 = (unsigned char)w;
    temp_v1 = (unsigned char)((w >> 8) & 0x01);
    tx[0] = 0x81;
    tx[1] = 0x6e;
    tx[2] = temp_v2;
    if (spi_send_data(tx, rx, 3) < 0)
    {
        return -1;
    }
    tx[0] = 0x81;
    tx[1] = 0x6f;
    tx[2] = temp_v1;
    if (spi_send_data(tx, rx, 3) < 0)
    {
        return -1;
    }

    temp_v2 = (unsigned char)height_base;
    temp_v1 = (unsigned char)((height_base >> 8) & 0x01);
    tx[0] = 0x81;
    tx[1] = 0x70;
    tx[2] = temp_v2;
    if (spi_send_data(tx, rx, 3) < 0)
    {
        return -1;
    }
    tx[0] = 0x81;
    tx[1] = 0x71;
    tx[2] = temp_v1;
    if (spi_send_data(tx, rx, 3) < 0)
    {
        return -1;
    }

    h = height_base + height - 1;
    temp_v2 = (unsigned char)h;
    temp_v1 = (unsigned char)((h >> 8) & 0x01);
    tx[0] = 0x81;
    tx[1] = 0x72;
    tx[2] = temp_v2;
    if (spi_send_data(tx, rx, 3) < 0)
    {
        return -1;
    }
    tx[0] = 0x81;
    tx[1] = 0x73;
    tx[2] = temp_v1;
    if (spi_send_data(tx, rx, 3) < 0)
    {
        return -1;
    }

    if (sensor_sleep() < 0)
    {
        return -1;
    }

    return 0;
}

static int sensor_init(int fd) // ISF_FULL_SHUT_12bit
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

    count = sizeof(init_config) / ISF_CONFIG_REG_LEN;
    for (i = 0; i < count; ++i)
    {
        ret = spi_send_data(init_config[i], rx, ISF_CONFIG_REG_LEN);
        if (ret < 0)
        {
            return -1;
        }
    }
    sensor_setImgWH(30, 30, SENSOR_WIDTH, SENSOR_HEIGHT);

    if (sensor_sleep() < 0)
    {
        return -1;
    }

    return 0;
}

static int sensor_setExpoTime(int time)
{
    unsigned char tx[3] = {0};
    unsigned char rx[3] = {0};
    unsigned int exp = time * 24000 / 1542;
    if (sensor_wakeup() < 0)
    {
        return -1;
    }

    tx[0] = 0x81;
    tx[1] = 0x92;
    tx[2] = exp & 0xff;
    if (spi_send_data(tx, rx, 3) < 0)
    {
        return -1;
    }

    tx[0] = 0x81;
    tx[1] = 0x93;
    tx[2] = (exp & 0x0000ff00) >> 8;
    if (spi_send_data(tx, rx, 3) < 0)
    {
        return -1;
    }

    tx[0] = 0x81;
    tx[1] = 0x94;
    tx[2] = (exp & 0x0000ff00) >> 16;
    if (spi_send_data(tx, rx, 3) < 0)
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
    if (gain > 7)
    {
        gain = 7;
        printf("The max gain value is 7!\n");
    }
    printf("==========gain=%d\n", gain);
    unsigned char tx[3] = {0};
    unsigned char rx[3] = {0};

    if (sensor_wakeup() < 0)
    {
        return -1;
    }

    tx[0] = 0x81;
    switch (gain)
    {
    case 0:
    {
        tx[1] = 0x28;
        tx[2] = 0x00;
        if (spi_send_data(tx, rx, 3) < 0)
        {
            return -1;
        }
        tx[1] = 0x2c;
        tx[2] = 0x07;
        if (spi_send_data(tx, rx, 3) < 0)
        {
            return -1;
        }
        break;
    }
    case 1:
    {
        tx[1] = 0x28;
        tx[2] = 0x10;
        if (spi_send_data(tx, rx, 3) < 0)
        {
            return -1;
        }
        tx[1] = 0x2c;
        tx[2] = 0x0b;
        if (spi_send_data(tx, rx, 3) < 0)
        {
            return -1;
        }
        break;
    }
    case 2:
    {
        tx[1] = 0x28;
        tx[2] = 0x20;
        if (spi_send_data(tx, rx, 3) < 0)
        {
            return -1;
        }
        tx[1] = 0x2c;
        tx[2] = 0x0d;
        if (spi_send_data(tx, rx, 3) < 0)
        {
            return -1;
        }
        break;
    }
    case 3:
    {
        tx[1] = 0x28;
        tx[2] = 0x30;
        if (spi_send_data(tx, rx, 3) < 0)
        {
            return -1;
        }
        tx[1] = 0x2c;
        tx[2] = 0x0e;
        if (spi_send_data(tx, rx, 3) < 0)
        {
            return -1;
        }
        break;
    }
    case 4:
    {
        tx[1] = 0x28;
        tx[2] = 0x40;
        if (spi_send_data(tx, rx, 3) < 0)
        {
            return -1;
        }
        tx[1] = 0x2c;
        tx[2] = 0x0e;
        if (spi_send_data(tx, rx, 3) < 0)
        {
            return -1;
        }
        break;
    }
    case 5:
    {
        tx[1] = 0x28;
        tx[2] = 0x50;
        if (spi_send_data(tx, rx, 3) < 0)
        {
            return -1;
        }
        tx[1] = 0x2c;
        tx[2] = 0x0e;
        if (spi_send_data(tx, rx, 3) < 0)
        {
            return -1;
        }
        break;
    }
    case 6:
    {
        tx[1] = 0x28;
        tx[2] = 0x60;
        if (spi_send_data(tx, rx, 3) < 0)
        {
            return -1;
        }
        tx[1] = 0x2c;
        tx[2] = 0x0e;
        if (spi_send_data(tx, rx, 3) < 0)
        {
            return -1;
        }
        break;
    }
    case 7:
    {
        tx[1] = 0x28;
        tx[2] = 0x70;
        if (spi_send_data(tx, rx, 3) < 0)
        {
            return -1;
        }
        tx[1] = 0x2c;
        tx[2] = 0x0e;
        if (spi_send_data(tx, rx, 3) < 0)
        {
            return -1;
        }
        break;
    }
    }
    if (sensor_sleep() < 0)
    {
        return -1;
    }

    return 0;
}

static int sensor_pre_image()
{
    unsigned char tx[3] = {0};
    unsigned char rx[3] = {0};

    if (sensor_wakeup() < 0)
    {
        return -1;
    }

    tx[0] = 0x81;
    tx[1] = 0x7d;
    tx[2] = 0x16;
    if (spi_send_data(tx, rx, 3) < 0)
    {
        printf("init failed");
        return -1;
    }

    tx[0] = 0x81;
    tx[1] = 0x98;
    tx[2] = 0x05;
    if (spi_send_data(tx, rx, 3) < 0)
    {
        printf("init failed");
        return -1;
    }

    tx[0] = 0x83;
    tx[1] = 0x41;
    tx[2] = 0x00;
    if (spi_send_data(tx, rx, 3) < 0)
    {
        printf("init failed");
        return -1;
    }

    tx[0] = 0x81;
    tx[1] = 0x6a;
    tx[2] = 0x01;
    if (spi_send_data(tx, rx, 3) < 0)
    {
        printf("start failed");
        return -1;
    }

    tx[0] = 0x81;
    tx[1] = 0x6a;
    tx[2] = 0x00;
    if (spi_send_data(tx, rx, 3) < 0)
    {
        printf("start failed");
        return -1;
    }

    return 0;
}

static int parse_raw_data(const unsigned char *raw, unsigned char *data)
{
    int ret = 0;
    const unsigned char *p = NULL;
    unsigned int size = 0;
    unsigned int byte_size = BYTE_SIZE; // MODE_12_BIT = 2  //MODE_8_BIT = 1
    unsigned int h = SENSOR_FULL_H;
    unsigned int w = SENSOR_FULL_W;
    unsigned int head_size = HEAD_SIZE;
    unsigned int line_num_size = LINE_NUM_SIZE;
    unsigned int hist_size = HIST_SIZE;
    unsigned int hist_m_size = HIST_M_SIZE;
    unsigned char line = 0x00;
    unsigned int i = 0;
    unsigned int hsel = 0;
    unsigned int sensor_head_sel = 0;
    unsigned char head[2] = {0x0A, 0xAA};

    size = (head_size + line_num_size + w) * h + (hist_size + head_size) + (hist_m_size + head_size);
    hsel = sensor_head_sel * byte_size;
    p = raw;
    for (i = 0; i < size * byte_size; i++)
    {
        if (p[i] != head[hsel] || p[i + 1] != head[hsel + 1])
        {
            printf("head err [0x%x][0x%x],cur line %d", p[i], p[i + 1], line);
            break;
        }
        i += head_size * byte_size;

        if (p[i] != 0x00 || p[i + 1] != line)
        {
            i -= head_size * byte_size;
            printf("line err [0x%x][0x%x], cur line %d", p[i], p[i + 1], line);
            break;
        }

        i += line_num_size * byte_size;
        cfp_memcpy(data + line * w * byte_size, (void *)(p + i), w * byte_size);
        i += w * byte_size;
        line += 1;
        i -= 1;
        if (line == h)
        {
            printf("read image success index %d\n", i);
            break;
        }
    }
    if (line == h)
    {
        i += 1;
        if (p[i] != head[hsel] || p[i + 1] != head[hsel + 1])
        {
            printf("head err [0x%x][0x%x]", p[i], p[i + 1]);
            ret = -1;
        }

        i += head_size * byte_size;
        cfp_memcpy(data + h * w * byte_size, (void *)(p + i), hist_size * byte_size);

        i += hist_size * byte_size;
        if (p[i] != head[hsel] || p[i + 1] != head[hsel + 1])
        {
            printf("head err [0x%x][0x%x]", p[i], p[i + 1]);
            ret = -1;
        }

        i += head_size * byte_size;
        cfp_memcpy(data + h * w * byte_size + hist_size, (void *)(p + i), hist_m_size * byte_size);
        i += hist_m_size * byte_size;
        if (i != size * byte_size)
        {
            printf("data num sync fail %d %d", i, size * byte_size);
            ret = -1;
            goto end;
        }
        printf("all data has been done %d %d\n", i, size * byte_size);
    }
    else
    {
        printf("parse image failed, line %d, index %d", line, i);
        ret = -1;
    }
end:
    return ret;
}

static int sensor_get_img_buffer(unsigned short *img_buff, int width, int height)
{
    unsigned char rx[6] = {0};
    unsigned char tx[6] = {0};
    unsigned char *p = NULL;
    unsigned int image_size = 0;
    unsigned int sensor_head_sel = 0;
    int i = 0, s = 0, ret;
    if (g_buffer == NULL)
        g_buffer = cfp_malloc(100 * 1024);
    cfp_memset(g_buffer, 0, 100 * 1024);
    if (g_image == NULL)
        g_image = cfp_malloc(100 * 1024);
    cfp_memset(g_image, 0, 100 * 1024);
    int sensor_buffer_size = (HEAD_SIZE + LINE_NUM_SIZE + SENSOR_FULL_W) * SENSOR_FULL_H + (HIST_SIZE + HEAD_SIZE) +
                             (HIST_M_SIZE + HEAD_SIZE);
    sensor_buffer_size = sensor_buffer_size * SENSOR_DOL;
    sensor_buffer_size = sensor_buffer_size * BYTE_SIZE + IMAGE_CMD_SIZE;
    tx[0] = 0x0f;
    tx[1] = 0x0f;
    tx[2] = 0x4b;
    tx[3] = 0x4b;
    tx[4] = 0x4b;
    tx[5] = 0x4b;
    cfp_memcpy(g_buffer, tx, 6);
    if (spi_send_data(g_buffer, g_buffer, sensor_buffer_size) < 0)
    {
        printf("get raw image failed");
        goto out;
    }

    tx[0] = 0x8e;
    tx[1] = 0x1e;
    tx[2] = 0x00;
    if (spi_send_data(tx, rx, 3) < 0)
    {
        printf("get clear irq failed");
        goto out;
    }

    image_size = sensor_buffer_size - IMAGE_CMD_SIZE;
    p = g_buffer + IMAGE_CMD_SIZE;
    ret = parse_raw_data(p, g_image);
    if (ret != 0) // timeout
    {
        printf("parse_raw_data failed %d\n", ret);
        goto out;
    }
    printf("image_size=%d\n", image_size);

    for (i = 0; i < (SENSOR_FULL_H * SENSOR_FULL_W); i++)
    {
        img_buff[i] = ((g_image[2 * i] << 8) | g_image[2 * i + 1]);
    }

    return ret;

out:
    cfp_free(g_buffer);
    g_buffer = NULL;
    cfp_free(g_image);
    g_image = NULL;
    return -1;
}

static int sensor_setFrameNum(uint32_t count)
{
    return 0;
}

static int sensor_config_process(char *img_path, int fd)
{
    return 0;
}

static int sensor_setBinning(int binning_mode)
{

    return 0;
}

void sensor_fps6037_isf1001_init(cdfinger_fops *sensor_fops)
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
}
