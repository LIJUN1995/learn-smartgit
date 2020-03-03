#ifndef _FINGERTECH_CAMERA_H
#define _FINGERTECH_CAMERA_H

extern chip_cfg camera_cfg;
extern chip_ptr camera_ptr;
extern void M1_V1_DelayMs(uint32_t ms);
status_t FingerTech_GetImg(void);
void Fingertech_cameragpioinit(void);
void Fingertech_camerainitresource(void);
void Fingertech_cameraconfig(void);


#endif
