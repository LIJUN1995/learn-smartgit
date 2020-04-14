#ifndef __FPS6038_GC07S0_H__
#define __FPS6038_GC07S0_H__

class Fps6038:public Cdfinger_fops{
private:
    uint8_t fusion_frame_config_value = 0;
    uint8_t sensor_power_flag = 0;
public:
    Fps6038();
    ~Fps6038();
    int sensor_init(void);
    bool sensor_verify_id(void);
    uint8_t read_register(uint16_t reg);
    int write_register(uint16_t reg, uint8_t value);
    int sensor_pre_image(void); 
    int sensor_get_img_buffer(std::vector<uint16_t> &vec);
    int sensor_setImgWH(int width_base, int height_base, int width, int height);
    int sensor_sleep(void);
    int sensor_wakeup(void);
    int sensor_setFrameNum(int count);
    int sensor_setExpoTime(int time);
    int sensor_setImgGain(uint8_t gain);
    int sensor_setBinning(int binning_mode);
};

#endif