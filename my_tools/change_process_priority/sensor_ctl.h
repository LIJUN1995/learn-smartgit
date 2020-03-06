#ifndef __CAMERA_CTL_H__
#define __CAMERA_CTL_H__

#include <stdbool.h>
#include <stdint.h>
#include <sys/ioctl.h>

typedef struct _cdfinger_spi_data
{
    unsigned char *rx;
    unsigned char *tx;
    int length;
    int tx_length;
} cdfinger_spi_data;

typedef struct _cdfinger_fops
{
    bool (*sensor_verify_id)(int);
    int (*sensor_init)(int fd);
    int (*sensor_pre_image)(void);
    int (*sensor_setExpoTime)(int time);
    int (*sensor_setImgGain)(unsigned char gain);
    int (*sensor_setFrameNum)(uint32_t count);
    int (*sensor_config_process)(char *img_path, int fd);
    int (*sensor_get_img_buffer)(unsigned short *img_buff, int width, int height);
    int (*sensor_setImgWH)(int width_base, int height_base, int width, int height);
    int (*sensor_setBinning)(int binning_mode);
    int (*sensor_wakeup)(void);
    int (*sensor_sleep)(void);
} cdfinger_fops;

#define CDFINGER_IOCTL_MAGIC_NO 0xFB
#define CDFINGER_INITERRUPT_MODE _IOW(CDFINGER_IOCTL_MAGIC_NO, 2, uint8_t)
#define CDFINGER_HW_RESET _IOW(CDFINGER_IOCTL_MAGIC_NO, 14, uint8_t)
#define CDFINGER_GET_STATUS _IO(CDFINGER_IOCTL_MAGIC_NO, 15)
#define CDFINGER_SPI_WRITE_AND_READ _IOWR(CDFINGER_IOCTL_MAGIC_NO, 18, cdfinger_spi_data)
#define CDFINGER_INIT_GPIO _IO(CDFINGER_IOCTL_MAGIC_NO, 20)
#define CDFINGER_INIT_IRQ _IO(CDFINGER_IOCTL_MAGIC_NO, 21)
#define CDFINGER_POWER_ON _IO(CDFINGER_IOCTL_MAGIC_NO, 22)
#define CDFINGER_RESET _IO(CDFINGER_IOCTL_MAGIC_NO, 23)
#define CDFINGER_RELEASE_DEVICE _IO(CDFINGER_IOCTL_MAGIC_NO, 25)
#define CDFINGER_WAKE_LOCK _IOW(CDFINGER_IOCTL_MAGIC_NO, 26, uint8_t)
#define CDFINGER_NEW_KEYMODE _IOW(CDFINGER_IOCTL_MAGIC_NO, 37, uint8_t)
#define CDFINGER_CONTROL_IRQ _IOW(CDFINGER_IOCTL_MAGIC_NO, 38, uint8_t)
#define CDFINGER_CHANGER_CLK_FREQUENCY _IOW(CDFINGER_IOCTL_MAGIC_NO, 39, uint32_t)
#define CDFINGER_TEST _IO(CDFINGER_IOCTL_MAGIC_NO, 40)
#define CDFINGER_CONTROL_RESET _IOW(CDFINGER_IOCTL_MAGIC_NO, 41, uint8_t)
#define CDFINGER_CONTROL_CS _IOW(CDFINGER_IOCTL_MAGIC_NO, 42, uint8_t)
#define CDFINGER_CONTROL_CLK _IOW(CDFINGER_IOCTL_MAGIC_NO, 43, uint8_t)
#define CDFINGER_SPI_ACTIVE _IO(CDFINGER_IOCTL_MAGIC_NO, 45)
#define CDFINGER_SPI_SLEEP _IO(CDFINGER_IOCTL_MAGIC_NO, 46)
#define CDFINGER_SPI_MODE _IOW(CDFINGER_IOCTL_MAGIC_NO, 47, uint8_t)
#define CDFINGER_SET_PRIO _IOW(CDFINGER_IOCTL_MAGIC_NO, 48, uint16_t)

extern int SENSOR_WIDTH;
extern int SENSOR_HEIGHT;

void fps6038_gc07s0_init(cdfinger_fops *sensor_fops);
void sensor_fps6037_isf1001_init(cdfinger_fops *sensor_fops);
void fps7001_gc0403_init(cdfinger_fops *sensor_fops);
void fps7001_gc0403_double_init(cdfinger_fops *sensor_fops);
void fps7011_gcm7s0_init(cdfinger_fops *sensor_fops);

#endif