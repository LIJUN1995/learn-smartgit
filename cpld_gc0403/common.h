#ifndef __COMMON_H__
#define __COMMON_H__

#define CDFINGER_IOCTL_MAGIC_NO 0xFB
#define CDFINGER_INITERRUPT_MODE _IOW(CDFINGER_IOCTL_MAGIC_NO, 2, uint8_t)
#define CDFINGER_HW_RESET _IOW(CDFINGER_IOCTL_MAGIC_NO, 14, uint8_t)
#define CDFINGER_GET_STATUS _IO(CDFINGER_IOCTL_MAGIC_NO, 15)
#define CDFINGER_SPI_WRITE_AND_READ _IOWR(CDFINGER_IOCTL_MAGIC_NO, 18, cdfinger_spi_data)
//#define CDFINGER_REPORT_KEY _IOW(CDFINGER_IOCTL_MAGIC_NO, 19, key_report_t)
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
#define CDFINGER_SET_PRIO _IOW(CDFINGER_IOCTL_MAGIC_NO, 48, uint8_t)

extern int m_fd;

extern int OpenDeviceFile(char *dir);
extern void DeviceInit(void);
extern void CloseDeviceFile(void);
int WaitSemCall(void);

#endif