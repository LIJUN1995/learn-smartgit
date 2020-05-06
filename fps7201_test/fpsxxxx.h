#ifndef __FPSXXXX_H__
#define __FPSXXXX_H__
class Cdfinger_fops{
protected:
    int private_fd;
    int img_width;
    int img_height;
public:
    virtual int sensor_init(void){return -1;}
    virtual int sensor_sleep(void){return -1;}
    virtual int sensor_wakeup(void){return -1;}
    virtual int sensor_pre_image(void){return -1;}
    virtual bool sensor_verify_id(void){return false;}
    virtual int sensor_setExpoTime(int time){return -1;}
    virtual int sensor_setFrameNum(int count){return -1;}
    virtual int sensor_setImgGain(uint8_t gain){return -1;}
    virtual int sensor_setBinning(int binning_mode){return -1;}
    virtual int sensor_get_img_buffer(std::vector<uint16_t> &vec){return -1;}
    virtual int sensor_setImgWH(int width_base, int height_base, int width, int height){return -1;}

    int get_private_fd(void);
    int spi_send_data(uint8_t *tx, uint8_t *rx, int len);
};

#endif