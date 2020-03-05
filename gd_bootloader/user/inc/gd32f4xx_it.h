/*!
    \file  gd32f4xx_it.h
    \brief the header file of the ISR
*/

/*
    Copyright (C) 2016 GigaDevice

    2016-08-15, V1.0.0, firmware for GD32F4xx
*/

#ifndef GD32F4XX_IT_H
#define GD32F4XX_IT_H

#include "gd32f4xx.h"
extern uint8_t dci_dma_receive_done;
extern uint8_t read_id;
extern uint8_t set_camera;
extern uint8_t spi_send_time;
extern uint8_t capture_picture;
extern uint8_t read_id_done;
extern uint8_t set_camera_done;
extern uint8_t capture_picture_done;
/* function declarations */
/* this function handles NMI exception */
void NMI_Handler(void);
/* this function handles HardFault exception */
void HardFault_Handler(void);
/* this function handles MemManage exception */
void MemManage_Handler(void);
/* this function handles BusFault exception */
void BusFault_Handler(void);
/* this function handles UsageFault exception */
void UsageFault_Handler(void);
/* this function handles SVC exception */
void SVC_Handler(void);
/* this function handles DebugMon exception */
void DebugMon_Handler(void);
/* this function handles PendSV exception */
void PendSV_Handler(void);
/* this function handles SysTick exception */
void SysTick_Handler(void);

#endif /* GD32F4XX_IT_H */
