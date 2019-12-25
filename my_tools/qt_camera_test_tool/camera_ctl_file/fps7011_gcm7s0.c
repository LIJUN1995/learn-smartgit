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
static unsigned char fusion_frame_config_value = 0;

static struct m_bit {
    unsigned long a : 8;
    unsigned long b : 8;
    unsigned long c : 4;
    unsigned long d : 4;
} * m_bit_p;

static unsigned short SENSOR_INIT_CONF[][2] = {
    {0x0330, 0xff}, {0x0330, 0x00}, {0x0330, 0xff}, {0x0330, 0x00}, {0x0330, 0xff}, {0x0330, 0x00}, {0x0300, 0x07},
    {0x0301, 0x23}, {0x0304, 0xc6}, {0x0305, 0x01}, {0x0302, 0x28}, {0x0306, 0x90}, {0x02ee, 0x30}, {0x0108, 0x06},
    {0x0109, 0xa0}, {0x0232, 0xc4}, {0x02d1, 0xd1}, {0x02cf, 0xa3}, {0x0221, 0x06}, {0x0229, 0x60}, {0x02ce, 0x6c},
    {0x0244, 0x20}, {0x0120, 0x01}, {0x016f, 0x01}, {0x0171, 0x11}, {0x0160, 0x25}, {0x0161, 0x06}, {0x0162, 0x00},
    {0x0163, 0x00}, {0x0164, 0x08}, {0x0287, 0x18}, {0x0297, 0xa3}, {0x017c, 0x28}, {0x0242, 0x9e}, {0x0243, 0x27},
    {0x010a, 0x00}, {0x010b, 0x08}, {0x010c, 0x00}, {0x010d, 0x04}, {0x010e, 0x05}, {0x010f, 0xa0}, {0x0119, 0x11},
    {0x0112, 0xe0}, {0x0113, 0x40}, {0x0117, 0x01}, {0x0118, 0x3f}, {0x0060, 0x00}, {0x0059, 0x00}, {0x0202, 0x01},
    {0x0203, 0x20}, {0x0202, 0x00}, {0x0203, 0x60}, {0x02b3, 0x00}, {0x0089, 0x03}, {0x2b0, 0x38},
};

static int spi_send_data(unsigned char *tx, unsigned char *rx, int len) {
    cdfinger_spi_data data;
    int ret = -1;

    data.rx = rx;
    data.tx = tx;
    data.length = len;
    data.tx_length = len;
    ret = ioctl(private_fd, CDFINGER_SPI_WRITE_AND_READ, &data);
    if (ret != 0) {
        printf("spi send data is error!!! ret = %d\n", ret);
    }

    return ret;
}

int write_register(unsigned short reg, unsigned char value) {
    unsigned char tx[4] = {0};
    unsigned char rx[4] = {0};

    // printf("0x%04x == 0x%02x",reg,value);

    tx[0] = 0xf0;
    tx[1] = reg >> 8;
    tx[2] = reg;
    tx[3] = value;
    if (spi_send_data(tx, rx, 4) < 0) {
        printf("write register failed");
        return -1;
    }

    return 0;
}

unsigned char read_register(unsigned short reg) {
    unsigned char tx[6] = {0};
    unsigned char rx[6] = {0};

    tx[0] = 0xf2;
    tx[1] = reg >> 8;
    tx[2] = reg;
    tx[3] = 0x00;
    tx[4] = 0x00;
    if (spi_send_data(tx, rx, 6) < 0) {
        printf("read register failed");
        return -1;
    }

    return rx[4];
}

static int sensor_sleep(void) {

    // unsigned char tx[3] = {0};
    // unsigned char rx[3] = {0};
    // if (sensor_power_flag != 0) {
    //     tx[0] = 0xfa;
    //     tx[1] = 0x04;
    //     tx[2] = 0x9b;
    //     if (spi_send_data(tx, rx, 3) < 0) {
    //         printf("sensor sleep failed!!!");
    //         return -1;
    //     }
    //     sensor_power_flag = 0;
    // }

    return 0;
}

static int sensor_wakeup(void) {
    // if (sensor_power_flag != 1) {
    //     unsigned char tx[3] = {0};
    //     unsigned char rx[3] = {0};

    //     tx[0] = 0xfa;
    //     tx[1] = 0xc4;
    //     tx[2] = 0x9b;
    //     if (spi_send_data(tx, rx, 3) < 0) {
    //         printf("sensor wakeup failed!!!");
    //         return -1;
    //     }
    //     sensor_power_flag = 1;
    // }
    return 0;
}

static bool sensor_verify_id(int fd) {
    unsigned char ret = 0;

    private_fd = fd;
    if (sensor_wakeup() < 0) {
        return false;
    }

    ret = read_register(0x3f0);
    if (ret != 0x27) {
        printf("fps7011 verify id failed=====%d!!!\n", ret);
        return false;
    }

    if (sensor_sleep() < 0) {
        return false;
    }

    return true;
}

static int sensor_setImgWH(int width_base, int height_base, int width, int height) {
    unsigned char temp = 0;

    if (sensor_wakeup() < 0) {
        return -1;
    }

    if (write_register(0x115, (unsigned char)width_base) < 0) {
        return -1;
    }

    if (write_register(0x116, (unsigned char)height_base) < 0) {
        return -1;
    }

    printf("width == 0x%08x,   height == 0x%08x,", width & 0x00000700, (height & 0x00000300) >> 8);
    temp = (width & 0x00000700) >> 4 | (height & 0x00000300) >> 8;
    if (write_register(0x119, temp) < 0) {
        return -1;
    }

    if (write_register(0x112, (unsigned char)width) < 0) {
        return -1;
    }

    if (write_register(0x113, (unsigned char)height) < 0) {
        return -1;
    }

    if (write_register(0x118, (unsigned char)height - 1) < 0) {
        return -1;
    }

    if (sensor_sleep() < 0) {
        return -1;
    }

    return 0;
}

static int sensor_init(int fd) {
    int ret = 0, i = 0;
    int count = 0;
    unsigned char rx[4] = {0};

    private_fd = fd;
    ioctl(private_fd, CDFINGER_INIT_GPIO);

    if (sensor_wakeup() < 0) {
        return -1;
    }

    count = sizeof(SENSOR_INIT_CONF) / sizeof(SENSOR_INIT_CONF[0]);
    for (i = 0; i < count; ++i) {
        ret = write_register(SENSOR_INIT_CONF[i][0], (unsigned char)SENSOR_INIT_CONF[i][1]);
        printf("0x%04x   0x%02x\n",SENSOR_INIT_CONF[i][0], SENSOR_INIT_CONF[i][1]);
        if (ret < 0) {
            return -1;
        }
    }

    for (i = 0; i < count; ++i) {
        unsigned char re = 0;
        re = read_register(SENSOR_INIT_CONF[i][0]);
        if (re != SENSOR_INIT_CONF[i][1]) {
            printf("0x%04x ++++++++++++++++++++++++++  0x%02x\n", SENSOR_INIT_CONF[i][0], re);
        }
    }

    if (sensor_sleep() < 0) {
        return -1;
    }

    return 0;
}

static int sensor_setFrameNum(uint32_t count) {
    unsigned char tx[4] = {0};
    unsigned char rx[4] = {0};
    unsigned char b = 0;

    if (sensor_wakeup() < 0) {
        return -1;
    }

    switch (count) {
    case 1:
        fusion_frame_config_value = 0x00;
        b = 0x02;
        break;

    case 2:
        fusion_frame_config_value = 0x80;
        b = 0x03;
        break;

    case 4:
        fusion_frame_config_value = 0x81;
        b = 0x05;
        break;

    case 8:
        fusion_frame_config_value = 0x82;
        b = 0x09;
        break;

    case 16:
        fusion_frame_config_value = 0x83;
        b = 0x11;
        break;

    default:
        printf("set fusion parameter failed,only(1,2,4,8,16)");
        return -1;
        break;
    }

    if (write_register(0x17b, fusion_frame_config_value) < 0) {
        return -1;
    }

    printf("============0x%02x===========0x%02x", b, fusion_frame_config_value);
    if (write_register(0x121, b) < 0) {
        return -1;
    }

    if (sensor_sleep() < 0) {
        return -1;
    }

    return 0;
}

static int sensor_setExpoTime(int time) {
    unsigned char temp = 0;
    unsigned char ret = 0;
    float row_time = 33.5;
    unsigned short exp = ((int)((time * 1000 / 33.5) * 3 - 12)) / 3 * 3;

    if (exp > 0x3fff) {
        exp = 0x3fff;
        printf("exp max_value is 0x3fff");
    }

    if (sensor_wakeup() < 0) {
        return -1;
    }

    temp = exp >> 8;
    if (write_register(0x202, temp) < 0) {
        return -1;
    }

    temp = (unsigned char)exp;
    if (write_register(0x203, temp) < 0) {
        return -1;
    }

    printf("==========exp=0x%04x\n", exp);

    ret = read_register(0x202);
    if (ret < 0) {
        return -1;
    }
    printf("==========0x%02x\n", ret);
    ret = read_register(0x203);
    if (ret < 0) {
        return -1;
    }
    printf("==========0x%02x\n", ret);

    if (sensor_sleep() < 0) {
        return -1;
    }

    return 0;
}

static int sensor_setImgGain(unsigned char gain) {
    unsigned char ret = 0;
    unsigned char temp_gain = 0;

    switch (gain) {
    case 1:
        temp_gain = 0x00;
        break;
    case 2:
        temp_gain = 0x01;
        break;
    case 3:
        temp_gain = 0x02;
        break;
    case 4:
        temp_gain = 0x03;
        break;
    case 5:
        temp_gain = 0x04;
        break;
    case 6:
        temp_gain = 0x0c;
        break;
    case 7:
        temp_gain = 0x14;
        break;
    case 8:
        temp_gain = 0x24;
        break;
    case 9:
        temp_gain = 0x2d;
        break;
    default:
        printf("set gain failed,the parameter error");
        break;
    }

    if (sensor_wakeup() < 0) {
        return -1;
    }

    if (write_register(0x2b3, temp_gain) < 0) {
        return -1;
    }

    // if (write_register(0x2b1, gain) < 0)
    // {
    //     return -1;
    // }
    // if (write_register(0x2b2, 0x00) < 0)
    // {
    //     return -1;
    // }

    printf("gain==0x%02x,  temp_gain==0x%02x\n", gain, temp_gain);

    ret = read_register(0x2b1);
    printf("read GAIN =========== 0x%02x\n", ret);

    if (sensor_sleep() < 0) {
        return -1;
    }

    return 0;
}

static int sensor_pre_image(void) {
    if (sensor_wakeup() < 0) {
        return -1;
    }

    if ((fusion_frame_config_value == 0x00) && (read_register(0x160) != 0x4f)) {
        // if (write_register(0x171, 0x14) < 0) {
        if (write_register(0x171, 0x11) < 0) {
            return -1;
        }
    } else {
        if (write_register(0x171, 0x11) < 0) {
            return -1;
        }
        if (write_register(0x330, 0x20) < 0) {
            return -1;
        }
        if (write_register(0x330, 0x00) < 0) {
            return -1;
        }
    }

    if (write_register(0x12f, 0x01) < 0) {
        return -1;
    }

    return 0;
}

static void convert_img(unsigned short *dst, unsigned char *src, int width, int height) {
    int len = width * height;
    int i = 0, j = 0;

    for (i = 0; i < len; i += 2) {
        m_bit_p = (struct m_bit *)(src + j * 3);
        dst[i] = ((m_bit_p->a << 4) | m_bit_p->d) & 0x0fff;
        dst[i + 1] = ((m_bit_p->b << 4) | m_bit_p->c) & 0x0fff;
        j++;
    }

    return;
}

static int sensor_get_img_buffer(unsigned short *img_buff, int width, int height)
// static int sensor_get_img_buffer(unsigned char *img_buff, int width, int height)
{
    unsigned char *tx = NULL;
    unsigned char *rx = NULL;
    int spi_len = width * height * 12 / 8 + 4;
    // int spi_len = width * height + 4;

    tx = (unsigned char *)malloc(spi_len);
    if (tx == NULL) {
        printf("malloc failed,data.tx==FULL!!!\n");
        return -1;
    }
    rx = (unsigned char *)malloc(spi_len);
    if (rx == NULL) {
        printf("malloc failed,data.rx==FULL!!!\n");
        free(tx);
        return -1;
    }

    memset(tx, 0x00, spi_len);
    memset(rx, 0x00, spi_len);

    if (write_register(0x17b, fusion_frame_config_value & 0x0f) < 0) {
        return -1;
    }

    if (write_register(0x171, 0x04) < 0) {
        goto out;
    }

    tx[0] = 0xf4;
    tx[1] = ((spi_len - 4) & 0xff00) >> 8;
    tx[2] = ((spi_len - 4) & 0x00ff);
    tx[3] = 0x00;
    if (spi_send_data(tx, rx, spi_len) < 0) {
        goto out;
    }

    if (write_register(0x16f, 0x00) < 0) {
        goto out;
    }

    if (write_register(0x17b, fusion_frame_config_value) < 0) {
        goto out;
    }

    convert_img(img_buff, rx + 4, width, height);
    // memcpy(img_buff,rx+4,spi_len-4);

    if (sensor_sleep() < 0) {
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

static int sensor_config_process(char *img_path, int fd) {

    return 0;
}

static int sensor_setBinning(int binning_mode) {
    unsigned char value = 0;

    if (sensor_wakeup() < 0) {
        return -1;
    }

    switch (binning_mode) {
    case 2:
        value = 0x25;
        sensor_setImgWH(0, 0, 480, 320);
        break;
    case 3:
        value = 0x3a;
        sensor_setImgWH(0, 0, 480, 318);
        break;
    case 4:
        value = 0x4f;
        sensor_setImgWH(0, 0, 480, 320);
        break;
    default:
        printf("set binning mode failed,the parameter error");
        return -1;
    }

    if (write_register(0x160, value) < 0) {
        return -1;
    }

    if (sensor_sleep() < 0) {
        return -1;
    }

    return 0;
}

void fps7011_gcm7s0_init(cdfinger_fops *sensor_fops) {
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
