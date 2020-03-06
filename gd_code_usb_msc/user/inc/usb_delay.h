/*!
    \file  usb_delay.h
    \brief usb delay driver header file
*/

/*
    Copyright (C) 2016 GigaDevice

    2016-10-19, V1.0.0, demo for GD32F4xx
*/

#ifndef USB_DELAY_H
#define USB_DELAY_H

#include "usb_core.h"

/* function declarations */
/* initializes delay unit using Timer2 */
void usb_timer_init (void);
/* delay in micro seconds */
void usb_udelay (const uint32_t usec);
/* delay in milli seconds */
void usb_mdelay (const uint32_t msec);

#endif /* USB_DELAY_H */
