#include <iostream>
#include <cstdint>
#include <iomanip>
#include <vector>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "fpsxxxx.h"
#include "fps6038_gc07s0.h"

using namespace std;

static uint16_t SENSOR_INIT_CONF[][2] = {
    {0x0330, 0xff}, {0x0330, 0x00}, {0x0330, 0xff}, {0x0330, 0x00},
    {0x0330, 0xff}, {0x0330, 0x00}, {0x0330, 0x20}, {0x0330, 0x00},
    {0x0304, 0xc6}, {0x0301, 0x10}, {0x0306, 0xcc}, {0x0305, 0x01},
    {0x0302, 0x18}, {0x02ee, 0x30}, {0x0219, 0x0a}, {0x010e, 0xb0},                                
    {0x0110, 0xd8}, {0x010a, 0x04}, {0x010c, 0x04}, {0x0112, 0xb0},
    {0x0113, 0xd8}, {0x0117, 0xd7}, {0x0290, 0x01}, {0x0248, 0x40},
    {0x0160, 0x08}, {0x0161, 0x06}, {0x0162, 0x00}, {0x0163, 0x00},
    {0x0087, 0x52}, {0x0120, 0x01}, {0x0121, 0x02}, {0x0164, 0x08},
    {0x0224, 0x7c}, {0x0272, 0x17}, {0x0232, 0xc8}, {0x0261, 0x8c},
    {0x0242, 0x3e}, {0x0262, 0x1c}, {0x02cf, 0xc2}, {0x0234, 0x22},
    {0x02cd, 0x25}, {0x0211, 0x20}, {0x0225, 0xa7}, {0x0221, 0x24}, 
    {0x0229, 0x24}, {0x0203, 0xf0}, {0x0089, 0x03}, {0x0060, 0x01},
    {0x016f, 0x01}, {0x0306, 0xc8}, {0x024e, 0x60}, {0x022c, 0x2c},
    {0x02ce, 0x60}, {0x023b, 0x0e}, {0x0225, 0x27}, {0x0211, 0x00},
    {0x0142, 0x3f}, {0x0143, 0x02}, {0x0144, 0x3f}, {0x0145, 0x02},
    {0x0226, 0x30}, {0x044c, 0x28}, {0x044d, 0x28}, {0x044e, 0x28},
    {0x044f, 0x28}, {0x0171, 0x14},
};

static struct m_bit {
    unsigned long a : 8;
    unsigned long b : 8;
    unsigned long c : 4;
    unsigned long d : 4;
} * m_bit_p;

static void convert_img(uint16_t *dst, uint8_t *src, int width, int height) {
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

Fps6038::Fps6038()
{
    cout<<"fps7011"<<endl;

    private_fd = open("/dev/fpsdev0", O_RDWR);
    if (private_fd < 0)
    {
        throw "open fpsdev0 fail";
    }

    ioctl(private_fd, CDFINGER_CHANGER_CLK_FREQUENCY, 9600000);
    ioctl(private_fd, CDFINGER_INIT_IRQ);
    ioctl(private_fd, CDFINGER_POWER_ON);
}

Fps6038::~Fps6038()
{
    close(private_fd);
    cout<<"~fps7011"<<endl;
}

uint8_t Fps6038::read_register(uint16_t reg)
{
    unsigned char tx[6] = {0};
    unsigned char rx[6] = {0};

    tx[0] = 0xf2;
    tx[1] = reg >> 8;
    tx[2] = reg;
    tx[3] = 0x00;
    tx[4] = 0x00;
    if (spi_send_data(tx, rx, 6) < 0) {
        cerr<<"read register failed"<<endl;
        return -1;
    }

    return rx[4];
}

 int Fps6038::write_register(unsigned short reg, unsigned char value) {
    unsigned char tx[4] = {0};
    unsigned char rx[4] = {0};

    tx[0] = 0xf0;
    tx[1] = reg >> 8;
    tx[2] = reg;
    tx[3] = value;
    if (spi_send_data(tx, rx, 4) < 0) {
        cerr<<"write register failed"<<endl;
        return -1;
    }

    return 0;
}


int Fps6038::sensor_sleep(void)
{
    // unsigned char tx[3] = {0};
    // unsigned char rx[3] = {0};
    // if (sensor_power_flag != 0)
    // {
    //     tx[0] = 0xfa;
    //     tx[1] = 0x04;
    //     tx[2] = 0x9b;
    //     if (spi_send_data(tx, rx, 3) < 0)
    //     {
    //         cerr<<"sensor sleep failed!!!"<<endl;
    //         return -1;
    //     }
    //     sensor_power_flag = 0;
    // }

    return 0;
}

int Fps6038::sensor_wakeup(void)
{
    // if (sensor_power_flag != 1)
    // {
    //     unsigned char tx[3] = {0};
    //     unsigned char rx[3] = {0};

    //     tx[0] = 0xfa;
    //     tx[1] = 0xc4;
    //     tx[2] = 0x9b;
    //     if (spi_send_data(tx, rx, 3) < 0)
    //     {
    //         cerr<<"sensor wakeup failed!!!"<<endl;
    //         return -1;
    //     }
    //     sensor_power_flag = 1;
    // }
    return 0;
}

bool Fps6038::sensor_verify_id(void)
{
    uint8_t id = 0;

    if (sensor_wakeup() < 0)
    {
        return false;
    }

    id = read_register(0x03f0);
    if (id != 0x07)
    {
        cerr<<"fps6038 verify id failed!!!"<<endl;
        return false;
    }

    if (sensor_sleep() < 0)
    {
        return false;
    }

    return true;
}

int Fps6038::sensor_setImgWH(int width_base, int height_base, int width, int height)
{
    int ret = 0;
    
    img_height = height;
    img_width = width;

    if (sensor_wakeup() < 0)
    {
        return -1;
    }

    ret = write_register(0x0115,width_base);
    if (ret < 0)
    {
        return -1;
    }

    ret = write_register(0x0116,height_base);
    if (ret < 0)
    {
        return -1;
    }

    ret = write_register(0x0112,width);
    if (ret < 0)
    {
        return -1;
    }

    ret = write_register(0x0113,height);
    if (ret < 0)
    {
        return -1;
    }

    ret = write_register(0x0117,height+height_base-1);
    if (ret < 0)
    {
        return -1;
    }

    if (sensor_sleep() < 0)
    {
        return -1;
    }

    return 0;
}

int Fps6038::sensor_init(void)
{
    int ret = 0;
    int count = 0;

    // if (sensor_wakeup() < 0) {
    //     return -1;
    // }

    cout<<hex;

    count = sizeof(SENSOR_INIT_CONF) / sizeof(SENSOR_INIT_CONF[0]);
    for (size_t i = 0; i < count; ++i) {
        ret = write_register(SENSOR_INIT_CONF[i][0], (unsigned char)SENSOR_INIT_CONF[i][1]);
        cout<<"0x"<<setw(4) << setfill('0')<<int(SENSOR_INIT_CONF[i][0]);
        cout<<"  0x"<<setw(2) << setfill('0')<<int(SENSOR_INIT_CONF[i][1])<<endl;
        if (ret < 0) {
            return -1;
        }
    }

    for (size_t i = 0; i < count; ++i) {
        unsigned char re = 0;
        re = read_register(SENSOR_INIT_CONF[i][0]);
        if (re != SENSOR_INIT_CONF[i][1]) {
            cout<<"0x"<<setw(4)<<setfill('0')<<int(SENSOR_INIT_CONF[i][0]);
            cout<<" !=  0x"<<setw(2)<<setfill('0')<<int(re)<<endl;
        }
    }

    cout<<dec<<endl;

    // if (sensor_sleep() < 0) {
    //     return -1;
    // }

    return 0;
}

int Fps6038::sensor_setExpoTime(int time)
{
    int ret = 0;
    uint16_t exp = time * 1000 / 59.077;

    if (sensor_wakeup() < 0)
    {
        return -1;
    }

    ret = write_register(0x0202, uint8_t(exp>>8));
    if (ret < 0)
    {
        return -1;
    }

    ret = write_register(0x0203, uint8_t(exp));
    if (ret < 0)
    {
        return -1;
    }

    cout<<"==========exp = "<<hex<<int(exp)<<dec<<endl;

    if (sensor_sleep() < 0)
    {
        return -1;
    }

    return 0;
}

int Fps6038::sensor_setImgGain(uint8_t gain)
{
    int ret = 0;
    uint8_t temp = 0;

    if (sensor_wakeup() < 0)
    {
        return -1;
    }

    if (gain <= 4)
    {
        temp = gain;
    }
    else if (gain == 5)
    {
        temp = 0x0c;
    }
    else if (gain >= 6)
    {
        temp = 0x14;
    }

    ret = write_register(0x02b3, temp);
    if (ret < 0)
    {
        return -1;
    }

    if (sensor_sleep() < 0)
    {
        return -1;
    }

    return 0;
}

int Fps6038::sensor_pre_image(void)
{
    int ret = 0;

    if (sensor_wakeup() < 0)
    {
        return -1;
    }

    ret = write_register(0x0171, 0x14);
    if (ret < 0)
    {
        return -1;
    }

    ret = write_register(0x012f, 0x01);
    if (ret < 0)
    {
        return -1;
    }

    return 0;
}

int Fps6038::sensor_get_img_buffer(vector<uint16_t> &vec)
{
    unsigned char *tx = NULL;
    unsigned char *rx = NULL;
    int ret = 0;
    int spi_len = img_width * img_height * 12 / 8 + 4;

    vec.resize(img_height*img_width);

    tx = new uint8_t[spi_len];
    if (tx == NULL)
    {
        cerr<<"malloc failed,data.tx==FULL!!!"<<endl;
        return -1;
    }
    rx = new uint8_t[spi_len];
    if (rx == NULL)
    {
        cerr<<"malloc failed,data.rx==FULL!!!"<<endl;
        delete tx;
        return -1;
    }

    ret = write_register(0x0171,0x04);
    if (ret < 0)
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

    ret = write_register(0x016f,0x00);
    if (ret < 0)
    {
        goto out;
    }
    convert_img(&vec[0], rx + 4, img_width, img_height);
   
    if (sensor_sleep() < 0)
    {
        return -1;
    }

    delete rx;
    delete tx;
    return 0;

out:
    delete rx;
    delete tx;
    return -1;
}