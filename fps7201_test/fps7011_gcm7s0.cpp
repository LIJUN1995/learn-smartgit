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
#include "fps7011_gcm7s0.h"
#include "DevicesOps.h"

using namespace std;

static uint16_t SENSOR_INIT_CONF[][2] = {
    /****binning2 240*160****/
    {0x0330, 0xff}, {0x0330, 0x00}, {0x0330, 0xff}, {0x0330, 0x00}, {0x0330, 0xff}, {0x0330, 0x00}, {0x0300, 0x07},
    {0x0301, 0x23}, {0x0304, 0xc6}, {0x0305, 0x01}, {0x0302, 0x28}, {0x0306, 0x90}, {0x02ee, 0x30}, {0x0108, 0x06},
    {0x0109, 0xa0}, {0x0232, 0xc4}, {0x02d1, 0xd1}, {0x02cf, 0xa3}, {0x0221, 0x06}, {0x0229, 0x60}, {0x02ce, 0x6c},
    {0x0244, 0x20}, {0x0120, 0x01}, {0x016f, 0x01}, {0x0171, 0x11}, {0x0160, 0x25}, {0x0161, 0x06}, {0x0162, 0x00},
    {0x0163, 0x00}, {0x0164, 0x08}, {0x0287, 0x18}, {0x0297, 0xa3}, {0x017c, 0x28}, {0x0242, 0x9e}, {0x0243, 0x27},
    {0x010a, 0x00}, {0x010b, 0x08}, {0x010c, 0x00}, {0x010d, 0x04}, {0x010e, 0x05}, {0x010f, 0xa0}, {0x0119, 0x11},
    {0x0112, 0xe0}, {0x0113, 0x40}, {0x0117, 0x01}, {0x0118, 0x3f}, {0x0060, 0x00}, {0x0059, 0x00}, {0x0202, 0x01},
    {0x0203, 0x20}, {0x0202, 0x05}, {0x0203, 0x60}, {0x02b3, 0x03}, {0x0089, 0x03}, {0x02b0, 0x38}, {0x0090, 0x01},
    {0x0091, 0x9e}, {0x0092, 0x68},
    /****64合一 360*120****/
    // {0x0330, 0xff}, {0x0330, 0x00}, {0x0330, 0xff}, {0x0330, 0x00}, {0x0330, 0xff}, {0x0330, 0x00}, {0x0300, 0x07},
    // {0x0301, 0x23}, {0x0302, 0x28}, {0x0304, 0xc6}, {0x0305, 0x01}, {0x0306, 0x93}, {0x0287, 0x18}, {0x010b, 0x04},
    // {0x010d, 0x03}, {0x010e, 0x05}, {0x010f, 0xa4}, {0x0111, 0xc4}, {0x0116, 0x00}, {0x0119, 0x10}, {0x0112, 0x68},
    // {0x0113, 0x78}, {0x0117, 0x00}, {0x0118, 0x77}, {0x02cf, 0xa3}, {0x0232, 0xc4}, {0x02d1, 0xd1}, {0x02ee, 0x30},
    // {0x0242, 0x9e}, {0x0243, 0x27}, {0x0221, 0x06}, {0x0229, 0x60}, {0x02ce, 0x6c}, {0x0244, 0x20}, {0x023d, 0xc0},
    // {0x023b, 0x18}, {0x023e, 0x00}, {0x0054, 0x81}, {0x0059, 0x00}, {0x04e0, 0x01}, {0x04e0, 0x00}, {0x0297, 0xb3},
    // {0x017c, 0x0d}, {0x0160, 0x80}, {0x0161, 0x06}, {0x0162, 0x00}, {0x0163, 0x00}, {0x0164, 0x08}, {0x0120, 0x01},
    // {0x016f, 0x01}, {0x0171, 0x11}, {0x0089, 0x03}, {0x0108, 0x0a}, {0x0109, 0xd0}, {0x0060, 0x00}, {0x0059, 0x00}, 
    // {0x0202, 0x00}, {0x0203, 0xe0}, {0x02b3, 0x03}, {0x0040, 0x93}, {0x044c, 0x00}, {0x044d, 0x00}, {0x044e, 0x00}, 
    // {0x044f, 0x00}, {0x0290, 0x00}, {0x0105, 0x05}, {0x0106, 0xd8}, 
    /****raw image 8bit 240*256****/
    // {0x0330, 0xff}, {0x0330, 0x00}, {0x0330, 0xff}, {0x0330, 0x00}, {0x0330, 0xff}, {0x0330, 0x00}, {0x0300, 0x07},
    // {0x0301, 0x23}, {0x0302, 0x28}, {0x0304, 0xc6}, {0x0305, 0x01}, {0x0306, 0x93}, {0x0287, 0x18}, {0x010b, 0x00},
    // {0x010d, 0x00}, {0x010e, 0x05}, {0x010f, 0xa8}, {0x0111, 0xc8}, {0x0116, 0x00}, {0x0119, 0x01}, {0x0112, 0xf0},
    // {0x0113, 0x00}, {0x0117, 0x00}, {0x0118, 0xff}, {0x02cf, 0xa3}, {0x0232, 0xc4}, {0x02d1, 0xd1}, {0x02ee, 0x30},
    // {0x0242, 0x9e}, {0x0243, 0x27}, {0x0221, 0x06}, {0x0229, 0x60}, {0x02ce, 0x6c}, {0x0244, 0x20}, {0x0297, 0x03},
    // {0x017c, 0x00}, {0x0160, 0x80}, {0x0161, 0x04}, {0x0162, 0x00}, {0x0163, 0x00}, {0x0164, 0x08}, {0x0120, 0x01},
    // {0x0171, 0x14}, {0x0089, 0x03}, {0x0108, 0x0a}, {0x0109, 0xd0}, {0x0060, 0x00}, {0x0059, 0x00}, {0x0202, 0x00},
    // {0x0203, 0xe0}, {0x02b3, 0x00}, {0x0040, 0x93}, {0x044c, 0x00}, {0x044d, 0x00}, {0x044e, 0x00}, {0x044f, 0x00},
    // {0x0290, 0x01}, {0x0202, 0x00}, {0x0203, 0xa0},
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

Fps7011::Fps7011()
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

Fps7011::~Fps7011()
{
    close(private_fd);
    cout<<"~fps7011"<<endl;
}

uint8_t Fps7011::read_register(uint16_t reg)
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

 int Fps7011::write_register(unsigned short reg, unsigned char value) {
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

bool Fps7011::sensor_verify_id(void)
{
    
    uint8_t ret = 0;

    // if (sensor_wakeup() < 0) {
    //     return false;
    // }

    ret = read_register(0x3f0);
    if (ret != 0x27) {
        cout<<"fps7011 verify id failed, id="<<hex<<int(ret)<<dec<<endl;
        return false;
    }

    // if (sensor_sleep() < 0) {
    //     return false;
    // }

    return true;
}

int Fps7011::sensor_init(void)
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

int Fps7011::sensor_pre_image(void) {
    if (write_register(0x171, 0x11) < 0) {
        return -1;
    }
    if (write_register(0x330, 0x20) < 0) {
        return -1;
    }
    if (write_register(0x330, 0x00) < 0) {
        return -1;
    }
    if (write_register(0x12f, 0x01) < 0) {
        return -1;
    }

    return 0;
}

int Fps7011::sensor_get_img_buffer(vector<uint16_t> &vec)
{
    vec.resize(img_height*img_width);
    
    uint8_t *tx = NULL;
    uint8_t *rx = NULL;

    int spi_len = img_height * img_width * 12 / 8 + 4;

    tx = new uint8_t[spi_len];
    if (tx == NULL) {
        cerr<<"malloc tx failed\n"<<endl;
        return -1;
    }
    rx = new uint8_t[spi_len];
    if (rx == NULL) {
        cerr<<"malloc rx failed\n"<<endl;
        delete tx;
        return -1;
    }

    if(write_register(0x17b, fusion_frame_config_value & 0x0f) < 0) {
         goto OUT;
    }

    if(write_register(0x171, 0x04) < 0) {
        goto OUT;
    }

    tx[0] = 0xf4;
    tx[1] = ((spi_len - 4) & 0xff00) >> 8;
    tx[2] = ((spi_len - 4) & 0x00ff);
    tx[3] = 0x00;
    if(spi_send_data(tx, rx, spi_len) < 0) {
        goto OUT;
    }

    if (write_register(0x16f, 0x00) < 0) {
        goto OUT;
    }

    if (write_register(0x17b, fusion_frame_config_value) < 0) {
        goto OUT;
    }

    convert_img(&vec[0],rx+4,img_width,img_height);
    
    delete tx;
    delete rx;
    return 0;
OUT:
    delete tx;
    delete rx;
    return 0;
}

int Fps7011::sensor_setImgWH(int width_base, int height_base, int width, int height)
{
    uint8_t temp = 0;

    img_width = width;
    img_height = height;

//     if (sensor_wakeup() < 0) {
//         return -1;
//     }

    if (write_register(0x115, (unsigned char)width_base) < 0) {
        return -1;
    }

    if (write_register(0x116, (unsigned char)height_base) < 0) {
        return -1;
    }

    // printf("width == 0x%08x,   height == 0x%08x,", width & 0x00000700, (height & 0x00000300) >> 8);
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

//     if (sensor_sleep() < 0) {
//         return -1;
//     }

    return 0;
}

int Fps7011::sensor_sleep(void) {

    unsigned char tx[3] = {0};
    unsigned char rx[3] = {0};
    if (sensor_power_flag != 0) {
        tx[0] = 0xfa;
        tx[1] = 0x04;
        tx[2] = 0x9b;
        if (spi_send_data(tx, rx, 3) < 0) {
            cout<<"sensor sleep failed!!!"<<endl;
            return -1;
        }
        sensor_power_flag = 0;
    }

    return 0;
}

int Fps7011::sensor_wakeup(void) {
    if (sensor_power_flag != 1) {
        unsigned char tx[3] = {0};
        unsigned char rx[3] = {0};

        tx[0] = 0xfa;
        tx[1] = 0xc4;
        tx[2] = 0x9b;
        if (spi_send_data(tx, rx, 3) < 0) {
            cout<<"sensor wakeup failed!!!"<<endl;
            return -1;
        }
        sensor_power_flag = 1;
    }
    return 0;
}

int Fps7011::sensor_setFrameNum(int count) {
    unsigned char temp = 0;

    if (sensor_wakeup() < 0) {
        return -1;
    }

    switch (count) {
    case 1:
        fusion_frame_config_value = 0x00;
        temp = 0x02;
        break;

    case 2:
        fusion_frame_config_value = 0x80;
        temp = 0x03;
        break;

    case 4:
        fusion_frame_config_value = 0x81;
        temp = 0x05;
        break;

    case 8:
        fusion_frame_config_value = 0x82;
        temp = 0x09;
        break;

    case 16:
        fusion_frame_config_value = 0x83;
        temp = 0x11;
        break;

    default:
        cerr<<"set fusion parameter failed,only(1,2,4,8,16)"<<endl;
        return -1;
        break;
    }

    if (write_register(0x17b, fusion_frame_config_value) < 0) {
        return -1;
    }

    if (write_register(0x121, temp) < 0) {
        return -1;
    }

    if (sensor_sleep() < 0) {
        return -1;
    }

    return 0;
}

int Fps7011::sensor_setExpoTime(int time) {
    unsigned char temp = 0;
    unsigned char ret = 0;

    unsigned short exp = ((int)((time * 1000 / 33.5) * 3 - 12)) / 3 * 3;

    if (exp > 0x3fff) {
        exp = 0x3fff;
        cout<<"exp max_value is 0x3fff"<<endl;
    }

    // if (sensor_wakeup() < 0) {
    //     return -1;
    // }

    temp = exp >> 8;
    if (write_register(0x202, temp) < 0) {
        return -1;
    }

    temp = (unsigned char)exp;
    if (write_register(0x203, temp) < 0) {
        return -1;
    }

    cout<<"exp = "<<hex<<int(exp)<<dec<<endl;

    // if (sensor_sleep() < 0) {
    //     return -1;
    // }

    return 0;
}

int Fps7011::sensor_setImgGain(uint8_t gain) {
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
        cerr<<"set gain failed,the parameter error"<<endl;
        break;
    }

    // if (sensor_wakeup() < 0) {
    //     return -1;
    // }

    if (write_register(0x2b3, temp_gain) < 0) {
        return -1;
    }

    // if (sensor_sleep() < 0) {
    //     return -1;
    // }

    return 0;
}

int Fps7011::sensor_setBinning(int binning_mode) {
    unsigned char value = 0;

    // if (sensor_wakeup() < 0) {
    //     return -1;
    // }

    switch (binning_mode) {
    case 2:
        value = 0x25;
        sensor_setImgWH(0, 0, 480, 320);
        img_width = 240;
        img_height = 160;
        break;
    case 3:
        value = 0x3a;
        sensor_setImgWH(0, 0, 480, 318);
        img_width = 160;
        img_height = 106;
        break;
    case 4:
        value = 0x4f;
        sensor_setImgWH(0, 0, 480, 320);
        img_width = 120;
        img_height = 80;
        break;
    default:
        cerr<<"set binning mode failed,the parameter error"<<endl;
        return -1;
    }

    if (write_register(0x160, value) < 0) {
        return -1;
    }

    // if (sensor_sleep() < 0) {
    //     return -1;
    // }

    return 0;
}

void Fps7011::write_otp(uint8_t addr, uint8_t value)
{   
    uint8_t ret = 0;
    ret = read_register(0x325);
    if(ret != 0){
        cerr<<"write OTP busy!!!!!!!!!"<<endl;
        return;
    }

    write_register(0x320,0x07);
    write_register(0x321,0x80);

    write_register(0x322,addr);
    write_register(0x323,value);

    // write_register(0x321,0xc0); //byte操作
    write_register(0x321,0xc8); //bit操作
}

uint8_t Fps7011::read_otp(uint8_t addr)
{   
    uint8_t ret = 0;
    ret = read_register(0x325);
    if(ret != 0){
        cerr<<"read OTP busy!!!!!!!!!"<<endl;
        // return 0;
    }

    write_register(0x320,0x07);
    write_register(0x321,0x80);

    write_register(0x322,addr);

    // write_register(0x321,0xa0); //byte操作
    write_register(0x321,0xa4); //bit操作

    return read_register(0x324);
} 

int Fps7011::sensor_test_otp(void)
{
    uint8_t ret = 0;

    for (size_t i = 0; i < 0xff; i++)
    {
        ret = read_otp(uint8_t(i));
        if(ret != 0){
            cerr<<"otp error! addr:"<<hex<<int(i)<<"   value:"<<int(ret)<<dec<<endl;
        }
    } 
    
    for (size_t i = 0; i < 0xff; i++)
    {
        if(i%2){
            write_otp(uint8_t(i),1);
            ret = read_otp(uint8_t(i));
            if(ret != 1){
                cerr<<"write otp error! addr:"<<hex<<int(i)<<"   value:"<<int(ret)<<dec<<endl;
            }
        }else{
            write_otp(uint8_t(i),0);
            ret = read_otp(uint8_t(i));
            if(ret != 0){
                cerr<<"write otp error! addr:"<<hex<<int(i)<<"   value:"<<int(ret)<<dec<<endl;
            }
        }
    }

    // for (size_t i = 0; i < 0xff; i+=8)
    // {
    //     ret = read_otp(uint8_t(i));
    //     if(ret != 0){
    //         cerr<<"otp error! addr:"<<hex<<int(i)<<"   value:"<<int(ret)<<dec<<endl;
    //     }
    // } 

    // int n = 0;
    // for (size_t i = 0; i < 0xff; i+=8)
    // {
    //     n++;
       
    //     write_otp(uint8_t(i),uint8_t(n));
    //     ret = read_otp(uint8_t(i));
    //     if(ret != uint8_t(n)){
    //         cerr<<"write otp error! addr:"<<hex<<int(i)<<"   value:"<<int(ret)<<dec<<endl;
    //     }
    // }

    return 0;
}