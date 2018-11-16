#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/ioport.h>
#include <linux/errno.h>
#include <linux/spi/spi.h>
#include <linux/interrupt.h>
#include <linux/irqreturn.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/wakelock.h>
#include <linux/kthread.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/spi/spidev.h>
#include <linux/semaphore.h>
#include <linux/poll.h>
#include <linux/fcntl.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/input.h>
#include <linux/signal.h>
#include <linux/spi/spi.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/gpio.h>
#include <linux/miscdevice.h>
//#include "mt_spi.h"
#include "../../../../spi/mediatek/mt6735/mt_spi.h"
#include <linux/fb.h>
#include <linux/notifier.h>
#include <linux/kfifo.h>
#include <linux/semaphore.h>
#include <mt-plat/mt_gpio.h>
#include <linux/irqflags.h>

static u8 cdfinger_debug = 0x01;
typedef struct key_report{
	int key;
	int value;
}key_report_t;

typedef struct _cdfinger_spi_data {
	unsigned char *tx;
	unsigned char *rx; 
	int length;
}cdfinger_spi_data;

#define CDFINGER_DBG(fmt, args...) \
	do{ \
		if(cdfinger_debug & 0x01) \
			printk( "[DBG][cdfinger]:%5d: <%s>" fmt, __LINE__,__func__,##args ); \
	}while(0)
#define CDFINGER_FUNCTION(fmt, args...) \
	do{ \
		if(cdfinger_debug & 0x02) \
			printk( "[DBG][cdfinger]:%5d: <%s>" fmt, __LINE__,__func__,##args ); \
	}while(0)
#define CDFINGER_REG(fmt, args...) \
	do{ \
		if(cdfinger_debug & 0x04) \
			printk( "[DBG][cdfinger]:%5d: <%s>" fmt, __LINE__,__func__,##args ); \
	}while(0)
#define CDFINGER_ERR(fmt, args...) \
    do{ \
		printk( "[DBG][cdfinger]:%5d: <%s>" fmt, __LINE__,__func__,##args ); \
    }while(0)

#define HAS_RESET_PIN
//#define DTS_PROBE
//#define ENABLE_SPI_FREQUENCY_CALIBRTION

#define VERSION                         "cdfinger version 3.0"
#define DEVICE_NAME                     "fpsdev0"
#define SPI_DRV_NAME                    "cdfinger"
#define FPS998							   0x70
#define FPS998E							   0x98
#define FPS980							   0x80
#define FPS956							   0x56

#define CDFINGER_IOCTL_MAGIC_NO          0xFB
#define CDFINGER_INIT                    _IOW(CDFINGER_IOCTL_MAGIC_NO, 0, uint8_t)
#define CDFINGER_GETIMAGE                _IOW(CDFINGER_IOCTL_MAGIC_NO, 1, uint8_t)
#define CDFINGER_INITERRUPT_MODE	     _IOW(CDFINGER_IOCTL_MAGIC_NO, 2, uint8_t)
#define CDFINGER_INITERRUPT_KEYMODE      _IOW(CDFINGER_IOCTL_MAGIC_NO, 3, uint8_t)
#define CDFINGER_INITERRUPT_FINGERUPMODE _IOW(CDFINGER_IOCTL_MAGIC_NO, 4, uint8_t)
#define CDFINGER_RELEASE_WAKELOCK        _IO(CDFINGER_IOCTL_MAGIC_NO, 5)
#define CDFINGER_CHECK_INTERRUPT         _IO(CDFINGER_IOCTL_MAGIC_NO, 6)
#define CDFINGER_SET_SPI_SPEED           _IOW(CDFINGER_IOCTL_MAGIC_NO, 7, uint8_t)
#define CDFINGER_CTL_SPI                        	_IOW(CDFINGER_IOCTL_MAGIC_NO,30,cdfinger_spi_data)
#define CDFINGER_GET_SCREEN_STATUS            _IOW(CDFINGER_IOCTL_MAGIC_NO,29,int)
#define CDFINGER_REPORT_KEY_LEGACY              _IOW(CDFINGER_IOCTL_MAGIC_NO, 10, uint8_t)
#define CDFINGER_REPORT_KEY              _IOW(CDFINGER_IOCTL_MAGIC_NO, 20, key_report_t)
#define CDFINGER_POWERDOWN               _IO(CDFINGER_IOCTL_MAGIC_NO, 11)
#define	CDFINGER_GETID					 _IO(CDFINGER_IOCTL_MAGIC_NO,12)
#define CDFINGER_HW_RESET               _IOW(CDFINGER_IOCTL_MAGIC_NO, 14, uint8_t)
#define CDFINGER_INITERRUPT_KEYMODE_NEW	     _IOW(CDFINGER_IOCTL_MAGIC_NO, 21, uint8_t)
#define CDFINGER_POLL_TRIGGER			 _IO(CDFINGER_IOCTL_MAGIC_NO,31)
#define KEY_INTERRUPT                   KEY_F11

//static int image_ready_num = 1;  // this is pre read image num , you can set min = 1 , max == 2.
//static int performance = 1;
//static bool cdfinger_flag = false;
enum work_mode {
	CDFINGER_MODE_NONE       = 1<<0,
	CDFINGER_INTERRUPT_MODE  = 1<<1,
	CDFINGER_KEY_MODE        = 1<<2,
	CDFINGER_FINGER_UP_MODE  = 1<<3,
	CDFINGER_READ_IMAGE_MODE = 1<<4,
	CDFINGER_MODE_MAX
};

enum spi_speed {
	CDFINGER_SPI_4M1 = 1,
	CDFINGER_SPI_4M4,
	CDFINGER_SPI_4M7,
	CDFINGER_SPI_5M1,
	CDFINGER_SPI_5M5,
	CDFINGER_SPI_6M1,
	CDFINGER_SPI_6M7,
	CDFINGER_SPI_7M4,
	CDFINGER_SPI_8M
};
enum chip_command {
	READ_IMAGE = 0X90,
	CHIP_RESET,
	CHIP_WORK,
};

typedef struct cdfinger_reg {
	u8	reg_rx[16];
	u8* reg21;
	u8 	reg21_len;
	u8*	reg22;
	u8 	reg22_len;
	u8*	reg27;
	u8 	reg27_len;
	u8 *image_agc;
}cdfinger_reg_t;

static struct cdfinger_data {
	struct spi_device *spi;
	struct mutex buf_lock;
	struct mutex transfer_lock;
	unsigned int irq;
	int irq_enabled;

	u8 *imagetxcmd;
	u8 *imagerxpix;
	u8 *imagebuf;
	int	imagewidth;
	int imageheight;
	int imagebufsize;
	int imagegetsize;
	u8	sensor_type;
	int header;
	cdfinger_reg_t reg;

	u32 vdd_ldo_enable;
	u32 vio_ldo_enable;
	u32 config_spi_pin;

	struct pinctrl *fps_pinctrl;
	struct pinctrl_state *fps_reset_high;
	struct pinctrl_state *fps_reset_low;
	struct pinctrl_state *fps_power_on;
	struct pinctrl_state *fps_power_off;
	struct pinctrl_state *fps_vio_on;
	struct pinctrl_state *fps_vio_off;
	struct pinctrl_state *cdfinger_spi_miso;
	struct pinctrl_state *cdfinger_spi_mosi;
	struct pinctrl_state *cdfinger_spi_sck;
	struct pinctrl_state *cdfinger_spi_cs;
	struct pinctrl_state *cdfinger_irq;

	int thread_wakeup;
	int wait_condition;
	int transfer_count;
	int process_interrupt;
	int interrupt_count;
	int key_report;
	enum work_mode device_mode;
	uint8_t int_count;
	cdfinger_spi_data s_data;
	struct timer_list int_timer;
	struct input_dev *cdfinger_inputdev;
	struct wake_lock cdfinger_lock;
	struct task_struct *cdfinger_thread;
#ifdef ENABLE_SPI_FREQUENCY_CALIBRTION
	struct task_struct *cdfinger_spi_calibration_thread;
#endif
	struct fasync_struct *async_queue;
	uint8_t cdfinger_interrupt;
	int check_time;
	u8 last_transfer;
	struct notifier_block notifier;
	struct kfifo image_fifo;
	u8 image_agc_ready;
}*g_cdfinger;
static DECLARE_WAIT_QUEUE_HEAD(cdfinger_waitqueue);
static DECLARE_WAIT_QUEUE_HEAD(waiter);
static DECLARE_WAIT_QUEUE_HEAD(cdfinger_wait);
//static u8 head_sync[4] = {0xaa,0xaa,0x0a,0xaa};
unsigned long int_flag;

static struct mt_chip_conf spi_conf = {
	.setuptime = 7,
	.holdtime = 7,
	.high_time = 13,
	.low_time = 13,
	.cs_idletime = 6,
	.cpol = 0,
	.cpha = 0,
	.rx_mlsb = 1,
	.tx_mlsb = 1,
	.tx_endian = 0,
	.rx_endian = 0,
	.com_mod = DMA_TRANSFER,
	.pause = 1,
	.finish_intr = 1,
	.deassert = 0,
	.ulthigh = 0,
	.tckdly = 0,
};

// static void cdfinger_disable_irq(struct cdfinger_data *cdfinger)
// {
// 	if(cdfinger->irq_enabled == 1)
// 	{
// 		disable_irq_nosync(cdfinger->irq);
// 		cdfinger->irq_enabled = 0;
// 		CDFINGER_DBG("irq disable\n");
// 	}
// }

// static void cdfinger_enable_irq(struct cdfinger_data *cdfinger)
// {
// 	if(cdfinger->irq_enabled == 0)
// 	{
// 		enable_irq(cdfinger->irq);
// 		cdfinger->irq_enabled =1;
// 		CDFINGER_DBG("irq enable\n");
// 	}
// }

static int cdfinger_getirq_from_platform(struct cdfinger_data *cdfinger)
{
	if(!(cdfinger->spi->dev.of_node)){
		CDFINGER_ERR("of node not exist!\n");
		return -1;
	}

	cdfinger->irq = irq_of_parse_and_map(cdfinger->spi->dev.of_node, 0);
	if(cdfinger->irq < 0)
	{
		CDFINGER_ERR("parse irq failed! irq[%d]\n",cdfinger->irq);
		return -1;
	}
	CDFINGER_DBG("get irq success! irq[%d]\n",cdfinger->irq);
	//pinctrl_select_state(cdfinger->fps_pinctrl, cdfinger->cdfinger_irq);
	return 0;
}
static int cdfinger_parse_dts(struct cdfinger_data *cdfinger)
{
	int ret = -1;

	if(cdfinger->spi == NULL)
	{
		CDFINGER_ERR("spi is NULL !\n");
		goto parse_err;
	}
	
#ifndef DTS_PROBE
	cdfinger->spi->dev.of_node = of_find_compatible_node(NULL,NULL,"cdfinger,fps1098");
#endif

	of_property_read_u32(cdfinger->spi->dev.of_node,"vdd_ldo_enable",&cdfinger->vdd_ldo_enable);
	of_property_read_u32(cdfinger->spi->dev.of_node,"vio_ldo_enable",&cdfinger->vio_ldo_enable);
	of_property_read_u32(cdfinger->spi->dev.of_node,"config_spi_pin",&cdfinger->config_spi_pin);

	CDFINGER_DBG("vdd_ldo_enable[%d], vio_ldo_enable[%d], config_spi_pin[%d]\n",
		cdfinger->vdd_ldo_enable, cdfinger->vio_ldo_enable, cdfinger->config_spi_pin);

	cdfinger->fps_pinctrl = devm_pinctrl_get(&cdfinger->spi->dev);
	if (IS_ERR(cdfinger->fps_pinctrl)) {
		ret = PTR_ERR(cdfinger->fps_pinctrl);
		CDFINGER_ERR("Cannot find fingerprint cdfinger->fps_pinctrl! ret=%d\n", ret);
		goto parse_err;
	}

	cdfinger->cdfinger_irq = pinctrl_lookup_state(cdfinger->fps_pinctrl,"finger_int_as_int");
	if (IS_ERR(cdfinger->cdfinger_irq))
	{
		ret = PTR_ERR(cdfinger->cdfinger_irq);
		CDFINGER_ERR("cdfinger->cdfinger_irq ret = %d\n",ret);
		goto parse_err;
	}

	cdfinger->fps_reset_low = pinctrl_lookup_state(cdfinger->fps_pinctrl,"finger_reset_en0");
	if (IS_ERR(cdfinger->fps_reset_low))
	{
		ret = PTR_ERR(cdfinger->fps_reset_low);
		CDFINGER_ERR("cdfinger->fps_reset_low ret = %d\n",ret);
		goto parse_err;
	}
	cdfinger->fps_reset_high = pinctrl_lookup_state(cdfinger->fps_pinctrl,"finger_reset_en1");
	if (IS_ERR(cdfinger->fps_reset_high))
	{
		ret = PTR_ERR(cdfinger->fps_reset_high);
		CDFINGER_ERR("cdfinger->fps_reset_high ret = %d\n",ret);
		goto parse_err;
	}

	if(cdfinger->config_spi_pin == 1)
	{
		cdfinger->cdfinger_spi_miso = pinctrl_lookup_state(cdfinger->fps_pinctrl,"fingerprint_spi_miso");
		if (IS_ERR(cdfinger->cdfinger_spi_miso))
		{
			ret = PTR_ERR(cdfinger->cdfinger_spi_miso);
			CDFINGER_ERR("cdfinger->cdfinger_spi_miso ret = %d\n",ret);
			goto parse_err;
		}
		cdfinger->cdfinger_spi_mosi = pinctrl_lookup_state(cdfinger->fps_pinctrl,"fingerprint_spi_mosi");
		if (IS_ERR(cdfinger->cdfinger_spi_mosi))
		{
			ret = PTR_ERR(cdfinger->cdfinger_spi_mosi);
			CDFINGER_ERR("cdfinger->cdfinger_spi_mosi ret = %d\n",ret);
			goto parse_err;
		}
		cdfinger->cdfinger_spi_sck = pinctrl_lookup_state(cdfinger->fps_pinctrl,"fingerprint_spi_sck");
		if (IS_ERR(cdfinger->cdfinger_spi_sck))
		{
			ret = PTR_ERR(cdfinger->cdfinger_spi_sck);
			CDFINGER_ERR("cdfinger->cdfinger_spi_sck ret = %d\n",ret);
			goto parse_err;
		}
		cdfinger->cdfinger_spi_cs = pinctrl_lookup_state(cdfinger->fps_pinctrl,"fingerprint_spi_cs");
		if (IS_ERR(cdfinger->cdfinger_spi_cs))
		{
			ret = PTR_ERR(cdfinger->cdfinger_spi_cs);
			CDFINGER_ERR("cdfinger->cdfinger_spi_cs ret = %d\n",ret);
			goto parse_err;
		}
	}

	if(cdfinger->vdd_ldo_enable == 1)
	{
		cdfinger->fps_power_on = pinctrl_lookup_state(cdfinger->fps_pinctrl,"fingerprint_power_high");
		if (IS_ERR(cdfinger->fps_power_on))
		{
			ret = PTR_ERR(cdfinger->fps_power_on);
			CDFINGER_ERR("cdfinger->fps_power_on ret = %d\n",ret);
			goto parse_err;
		}

		cdfinger->fps_power_off = pinctrl_lookup_state(cdfinger->fps_pinctrl,"fingerprint_power_low");
		if (IS_ERR(cdfinger->fps_power_off))
		{
			ret = PTR_ERR(cdfinger->fps_power_off);
			CDFINGER_ERR("cdfinger->fps_power_off ret = %d\n",ret);
			goto parse_err;
		}
	}

	if(cdfinger->vio_ldo_enable == 1)
	{
		cdfinger->fps_vio_on = pinctrl_lookup_state(cdfinger->fps_pinctrl,"fingerprint_vio_high");
		if (IS_ERR(cdfinger->fps_vio_on))
		{
			ret = PTR_ERR(cdfinger->fps_vio_on);
			CDFINGER_ERR("cdfinger->fps_vio_on ret = %d\n",ret);
			goto parse_err;
		}

		cdfinger->fps_vio_off = pinctrl_lookup_state(cdfinger->fps_pinctrl,"fingerprint_vio_low");
		if (IS_ERR(cdfinger->fps_vio_off))
		{
			ret = PTR_ERR(cdfinger->fps_vio_off);
			CDFINGER_ERR("cdfinger->fps_vio_off ret = %d\n",ret);
			goto parse_err;
		}
	}

	return 0;
parse_err:
	CDFINGER_ERR("parse dts failed!\n");

	return ret;
}

static int spi_send_cmd(struct cdfinger_data *cdfinger,  u8 *tx, u8 *rx, u16 spilen)
{
	struct spi_message m;
	struct mt_chip_conf *spiconf = &spi_conf;
	struct spi_transfer t = {
		.tx_buf = tx,
		.rx_buf = rx,
		.len = spilen,
	};

	CDFINGER_DBG("transfer msg[0x%x]\n", tx[0]);

	if(spilen > 8)
	{
		if(spiconf->com_mod != DMA_TRANSFER)
		{
			spiconf->com_mod = DMA_TRANSFER;
			spi_setup(cdfinger->spi);
		}
	}
	else
	{
		if(spiconf->com_mod != FIFO_TRANSFER)
		{
			spiconf->com_mod = FIFO_TRANSFER;
			spi_setup(cdfinger->spi);
		}
	}

	spi_message_init(&m);
	spi_message_add_tail(&t, &m);

	return spi_sync(cdfinger->spi, &m);
}

static int spi_send_cmd_fifo(struct cdfinger_data *cdfinger,  u8 *tx, u8 *rx, u16 spilen)
{
	int ret = 0;

	ret = spi_send_cmd(cdfinger, tx, rx, spilen);

	udelay(100);

	return ret;
}

static void cdfinger_power_on(struct cdfinger_data *cdfinger)
{
	if(cdfinger->config_spi_pin == 1)
	{
		pinctrl_select_state(cdfinger->fps_pinctrl, cdfinger->cdfinger_spi_miso);
		pinctrl_select_state(cdfinger->fps_pinctrl, cdfinger->cdfinger_spi_mosi);
		pinctrl_select_state(cdfinger->fps_pinctrl, cdfinger->cdfinger_spi_sck);
		pinctrl_select_state(cdfinger->fps_pinctrl, cdfinger->cdfinger_spi_cs);
	}

	if(cdfinger->vdd_ldo_enable == 1)
	{
		pinctrl_select_state(cdfinger->fps_pinctrl, cdfinger->fps_power_on);
	}

	if(cdfinger->vio_ldo_enable == 1)
	{
		pinctrl_select_state(cdfinger->fps_pinctrl, cdfinger->fps_vio_on);
	}
}

static void cdfinger_power_off(struct cdfinger_data *cdfinger)
{
	if(cdfinger->vdd_ldo_enable == 1)
	{
		pinctrl_select_state(cdfinger->fps_pinctrl, cdfinger->fps_power_off);
	}

	if(cdfinger->vio_ldo_enable == 1)
	{
		pinctrl_select_state(cdfinger->fps_pinctrl, cdfinger->fps_vio_off);
	}
}

static int cdfinger_spi_send_data(struct cdfinger_data *cdfinger, unsigned long arg)
{
	int re_count = -1,status = -1,t_out = -1;
	cdfinger_spi_data  __user *cds_data = (cdfinger_spi_data  __user *)arg;
	CDFINGER_DBG("lj start gpio_status = %d\n",mt_get_gpio_in(1));

	cdfinger->s_data.length = cds_data->length;
	//memset(cdfinger->s_data.rx,0,cdfinger->s_data.length);
	//cdfinger->s_data.rx = (unsigned char *)kzalloc(cdfinger->s_data.length,GFP_KERNEL);
	re_count = copy_from_user(cdfinger->s_data.tx,cds_data->tx,8);
	CDFINGER_DBG("lj cdfinger->s_data.tx[0] = 0x%02x\n",cdfinger->s_data.tx[0]);
	status = spi_send_cmd_fifo(cdfinger,cdfinger->s_data.tx,cdfinger->s_data.rx,8);
	t_out = wait_event_interruptible_timeout(cdfinger_wait, cdfinger->wait_condition != 0,1000);
	CDFINGER_DBG("lj t_out = %d\n",t_out);
	re_count = copy_to_user(cds_data->rx,cdfinger->s_data.rx,cdfinger->s_data.length);
	cdfinger->wait_condition = 0;
	memset(cdfinger->s_data.tx,0,8);
	//kfree(cdfinger->s_data.rx);

/*	//cdfinger_spi_data  __user *cds_data = (cdfinger_spi_data  __user *)arg;
	//CDFINGER_DBG("lj start gpio_status = %d\n",mt_get_gpio_in(1));
	cdfinger->s_data.length = cds_data->length;
	cdfinger->s_data.tx = (unsigned char *)kzalloc(cdfinger->s_data.length,GFP_KERNEL);
	cdfinger->s_data.rx = (unsigned char *)kzalloc(cdfinger->s_data.length,GFP_KERNEL);
	ret = copy_from_user(cdfinger->s_data.tx,cds_data->tx,cdfinger->s_data.length);
	i = spi_send_cmd_fifo(cdfinger,cdfinger->s_data.tx,cdfinger->s_data.rx,cdfinger->s_data.length);
	// for(i = 0;i < cdfinger->s_data.length;i++)
	// CDFINGER_DBG("rx[%d] = 0x%02x\n",i,cdfinger->s_data.rx[i]);
	ret = copy_to_user(cds_data->rx,cdfinger->s_data.rx,cdfinger->s_data.length);
	kfree(cdfinger->s_data.tx);
	kfree(cdfinger->s_data.rx);
*/
	CDFINGER_DBG("lj end gpio_status = %d\n",mt_get_gpio_in(1));
	if(t_out == 0){
		CDFINGER_DBG("ioctl wait timeout\n");
		return 0;
	}else{
		return re_count;
	}
}

static int cdfinger_set_spi_speed(struct cdfinger_data *cdfinger, uint8_t arg)
{
	struct mt_chip_conf *spi_par = &spi_conf;
	enum spi_speed speed = arg;

	switch(speed){
		case CDFINGER_SPI_4M1:
			spi_par->high_time = 16;
			spi_par->low_time = 16;
			break;
		case CDFINGER_SPI_4M4:
			spi_par->high_time = 15;
			spi_par->low_time = 15;
			break;
		case CDFINGER_SPI_4M7:
			spi_par->high_time = 14;
			spi_par->low_time = 14;
			break;
		case CDFINGER_SPI_5M1:
			spi_par->high_time = 13;
			spi_par->low_time = 13;
			break;
		case CDFINGER_SPI_5M5:
			spi_par->high_time = 12;
			spi_par->low_time = 12;
			break;
		case CDFINGER_SPI_6M1:
			spi_par->high_time = 11;
			spi_par->low_time = 11;
			break;
		case CDFINGER_SPI_6M7:
			spi_par->high_time = 10;
			spi_par->low_time = 10;
			break;
		case CDFINGER_SPI_7M4:
			spi_par->high_time = 9;
			spi_par->low_time = 9;
			break;
		case CDFINGER_SPI_8M:
			spi_par->high_time = 8;
			spi_par->low_time = 8;
			break;
		default:
			return -ENOTTY;
	}

	CDFINGER_DBG("spi high_time[%d],low_time[%d]\n",spi_par->high_time,spi_par->low_time);

	return spi_setup(cdfinger->spi);
}


static long cdfinger_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct cdfinger_data *cdfinger = filp->private_data;
	int ret = 0;
    printk(KERN_INFO "Current :\n pid : %d\n",current->pid);/*当前进程PID*/
    printk(KERN_INFO "Task state: %ld\n",current->state);/*运行状态，-1为不可运行，0为可运行，>0为运行结束*/
    printk(KERN_INFO "Task name: %s\n",current->comm);/*进程名*/

	CDFINGER_FUNCTION("enter\n");
	if(cdfinger == NULL)
	{
		CDFINGER_ERR("%s: fingerprint please open device first!\n", __func__);
		return -EIO;
	}

	switch (cmd) {
		case CDFINGER_CTL_SPI:
			ret = cdfinger_spi_send_data(cdfinger,arg);
			break;
		case CDFINGER_SET_SPI_SPEED:
			ret = cdfinger_set_spi_speed(cdfinger,arg);
			break;
		default:
			ret = -ENOTTY;
			break;
	}
	CDFINGER_FUNCTION("exit\n");

	return ret;
}

static int cdfinger_open(struct inode *inode, struct file *file)
{
	CDFINGER_FUNCTION("enter\n");
	file->private_data = g_cdfinger;
	CDFINGER_FUNCTION("exit\n");

	return 0;
}

static ssize_t cdfinger_write(struct file *file, const char *buff, size_t count, loff_t * ppos)
{
	return 0;
}

static int cdfinger_async_fasync(int fd, struct file *filp, int mode)
{
	return 0;
}

static ssize_t cdfinger_read(struct file *file, char *buff, size_t count, loff_t * ppos)
{
	return 0;
}

static int cdfinger_release(struct inode *inode, struct file *file)
{
	struct cdfinger_data *cdfinger = file->private_data;

	CDFINGER_FUNCTION("enter\n");
	if(cdfinger == NULL)
	{
		CDFINGER_ERR("%s: fingerprint please open device first!\n", __func__);
		return -EIO;
	}
	file->private_data = NULL;
	CDFINGER_FUNCTION("exit\n");

	return 0;
}

static const struct file_operations cdfinger_fops = {
	.owner = THIS_MODULE,
	.open = cdfinger_open,
	.write = cdfinger_write,
	.read = cdfinger_read,
	.release = cdfinger_release,
	.fasync = cdfinger_async_fasync,
	.unlocked_ioctl = cdfinger_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = cdfinger_ioctl,
#endif
};

static struct miscdevice cdfinger_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &cdfinger_fops,
};

static int cdfinger_thread_func(void *arg)
{
	int ret = -1,count = 0;
	int i = 0,num = 0;
	//int i,j;
	unsigned char *tx = NULL;
	unsigned char *rx = NULL;
	struct cdfinger_data *cdfinger = (struct cdfinger_data *)arg;
	tx = (unsigned char *)kzalloc(cdfinger->transfer_count,GFP_KERNEL);
	rx = (unsigned char *)kzalloc(cdfinger->transfer_count,GFP_KERNEL);
	memset(tx,0x66,cdfinger->transfer_count);
	 do {
	 	wait_event_interruptible(waiter, cdfinger->thread_wakeup != 0);
		CDFINGER_DBG("lj wait event end ;cdfinger->s_data.tx[0] = 0x%02x\n",cdfinger->s_data.tx[0]);
		switch(cdfinger->s_data.tx[0]) {
			case READ_IMAGE: {
					if(cdfinger->s_data.length%1024 == 0)
						count = cdfinger->s_data.length/1024;
					else 
						count = cdfinger->s_data.length/1024+1;
					CDFINGER_DBG("lj count ============ %d\n",count*1024);
					ret = spi_send_cmd_fifo(cdfinger,tx,rx,count*1024);
					num = 0;
					for(i = 0;i < count*1024;i++)
						if(rx[i] == 0x95)
							num++;
					CDFINGER_DBG("lj num1 ============ %d\n",num);	
					num = 0;
					for(i = 0;i < cdfinger->s_data.length;i++)
						if(rx[i] == 0x95)
							num++;
					CDFINGER_DBG("lj num2 ============ %d\n",num);	
					memcpy(cdfinger->s_data.rx,rx,cdfinger->s_data.length);
					memset(rx,0,cdfinger->transfer_count);
			}
			break;
			case CHIP_RESET: {

			}
			break;
			case CHIP_WORK: {

			}
			break;
			default:
				goto err_command;
		}

		cdfinger->wait_condition = 1;
		wake_up_interruptible(&cdfinger_wait);
	err_command:
		cdfinger->thread_wakeup = 0;
	 }while(1);
	return 0;
}

static irqreturn_t cdfinger_interrupt_handler(unsigned irq, void *arg)
{
	struct cdfinger_data *cdfinger = g_cdfinger;
	cdfinger->thread_wakeup = 1;
	wake_up_interruptible(&waiter);
	return IRQ_HANDLED;
}

static int cdfinger_probe(struct spi_device *spi)
{
	struct cdfinger_data *cdfinger = NULL;
	int status = -ENODEV;

	CDFINGER_DBG("enter\n");
	cdfinger = kzalloc(sizeof(struct cdfinger_data), GFP_KERNEL);
	if (!cdfinger) {
		CDFINGER_ERR("alloc cdfinger failed!\n");
		return -ENOMEM;;
	}

	g_cdfinger = cdfinger;
	cdfinger->spi = spi;
	if(cdfinger_parse_dts(cdfinger))
	{
		CDFINGER_ERR("%s: parse dts failed!\n", __func__);
		goto free_cdfinger;
	}

	spi->bits_per_word = 8;
	spi->mode = SPI_MODE_0;
	spi->controller_data = (void *)&spi_conf;
	if(spi_setup(spi) != 0)
	{
		CDFINGER_ERR("%s: spi setup failed!\n", __func__);
		goto free_cdfinger;
	}

	cdfinger_power_on(cdfinger);
	//cdfinger_reset(100);

	cdfinger->thread_wakeup = 0;
	cdfinger->wait_condition = 0;
	cdfinger->transfer_count = 1024*30;

	cdfinger->s_data.tx = (unsigned char *)kzalloc(8,GFP_KERNEL);
	if (!cdfinger->s_data.tx)
	{
		CDFINGER_ERR("%s: cdfinger->s_data.tx malloc fail!\n", __func__);
		goto free_tx;
	}

	cdfinger->s_data.rx = (unsigned char *)kzalloc(cdfinger->transfer_count,GFP_KERNEL);
	if (!cdfinger->s_data.rx)
	{
		CDFINGER_ERR("%s: cdfinger->s_data.rx malloc fail!\n", __func__);
		goto free_rx;
	}

	status = misc_register(&cdfinger_dev);
	if (status < 0) {
		CDFINGER_ERR("%s: cdev register failed!\n", __func__);
		goto free_device;
	}


	if(cdfinger_getirq_from_platform(cdfinger)!=0)
		goto free_device;

	status = request_threaded_irq(cdfinger->irq, (irq_handler_t)cdfinger_interrupt_handler, NULL,
					IRQF_TRIGGER_RISING | IRQF_NO_SUSPEND, "cdfinger-irq", cdfinger);
	if(status){
		CDFINGER_ERR("request_irq error\n");
		goto free_device;
	}

	enable_irq_wake(cdfinger->irq);
	cdfinger->irq_enabled = 1;

	cdfinger->cdfinger_thread = kthread_run(cdfinger_thread_func, cdfinger, "cdfinger_thread");
	if (IS_ERR(cdfinger->cdfinger_thread)) {
		CDFINGER_ERR("kthread_run is failed\n");
		goto free_irq;
	}

	CDFINGER_DBG("exit\n");

	return 0;

free_irq:
	free_irq(cdfinger->irq, cdfinger);
free_device:
	misc_deregister(&cdfinger_dev);
free_rx:
	kfree(cdfinger->s_data.rx);
	cdfinger->s_data.rx = NULL;
free_tx:
	kfree(cdfinger->s_data.tx);
	cdfinger->s_data.tx = NULL;
	cdfinger_power_off(cdfinger);
free_cdfinger:
	kfree(cdfinger);
	cdfinger = NULL;

	return -1;
}

static int cdfinger_suspend (struct device *dev)
{
	return 0;
}

static int cdfinger_resume (struct device *dev)
{
	return 0;
}

static int cdfinger_remove(struct spi_device *spi)
{
	struct cdfinger_data *cdfinger = spi_get_drvdata(spi);

	kthread_stop(cdfinger->cdfinger_thread);
	free_irq(cdfinger->irq, cdfinger);
	misc_deregister(&cdfinger_dev);
	kfree(cdfinger->s_data.rx);
	cdfinger->s_data.rx = NULL;
	kfree(cdfinger->s_data.tx);
	cdfinger->s_data.tx = NULL;
	kfree(cdfinger);
	cdfinger = NULL;
	g_cdfinger = NULL;
	cdfinger_power_off(cdfinger);

	return 0;
}

static const struct dev_pm_ops cdfinger_pm = {
	.suspend = cdfinger_suspend,
	.resume = cdfinger_resume
};

struct of_device_id cdfinger_of_match[] = {
	{ .compatible = "cdfinger,fps998e", },
	{ .compatible = "cdfinger,fps1098", },
	{ .compatible = "cdfinger,fps998", },
	{ .compatible = "cdfinger,fps980", },
	{ .compatible = "cdfinger,fps956", },
	{},
};
MODULE_DEVICE_TABLE(of, cdfinger_of_match);

static const struct spi_device_id cdfinger_id[] = {
	{SPI_DRV_NAME, 0},
	{}
};
MODULE_DEVICE_TABLE(spi, cdfinger_id);

static struct spi_driver cdfinger_driver = {
	.driver = {
		.name = SPI_DRV_NAME,
		.bus = &spi_bus_type,
		.owner = THIS_MODULE,
		.pm = &cdfinger_pm,
		.of_match_table = of_match_ptr(cdfinger_of_match),
	},
	.id_table = cdfinger_id,
	.probe = cdfinger_probe,
	.remove = cdfinger_remove,
};

#ifndef DTS_PROBE 
static struct spi_board_info spi_board_cdfinger[] __initdata = {
	[0] = {
		.modalias = "cdfinger",
		.bus_num = 0,
		.chip_select = 0,
		.mode = SPI_MODE_0,
		.max_speed_hz = 6000000,
	},
};
#endif

static int cdfinger_spi_init(void)
{
#ifndef DTS_PROBE 
	spi_register_board_info(spi_board_cdfinger, ARRAY_SIZE(spi_board_cdfinger));
#endif

	return spi_register_driver(&cdfinger_driver);
}

static void cdfinger_spi_exit(void)
{
	spi_unregister_driver(&cdfinger_driver);
}

late_initcall_sync(cdfinger_spi_init);
module_exit(cdfinger_spi_exit);

MODULE_DESCRIPTION("cdfinger spi Driver");
MODULE_AUTHOR("shuaitao@cdfinger.com");
MODULE_LICENSE("GPL");
MODULE_ALIAS("cdfinger");
