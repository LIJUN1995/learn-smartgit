#ifndef CAMERA_H_
#define CAMERA_H_
                               
extern uint8_t raw_img_tx[4];
extern uint8_t raw_img_rx[4];


#pragma pack(push)
#pragma pack(1)
typedef struct chip_cfg_t
{
	uint32_t x;
	uint32_t y;
	uint32_t stat;
	uint32_t update;
	uint32_t width;
	uint32_t hight;
	uint32_t exp_time;
	uint32_t full_size_w;  
	uint32_t update_program;
	uint32_t camera_id;
}chip_cfg;
#pragma pack(pop)

void camera_init(void);
void camera_pre_image(uint8_t val);
void camera_get_image(void);
bool camera_verify_id(void) ;
void camera_reset_pin_init(void);


#endif
