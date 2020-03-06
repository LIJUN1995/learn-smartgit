#ifndef _FINGERTECH_SP0828_H
#define _FINGERTECH_SP0828_H

void Fingertech_sp0828gpioinit(void);
void Fingertech_sp0828iicconfig(void);
void Fingertech_sp0828iicwritedata(uint16_t regaddress,uint8_t data);
void Fingertech_sp0828iicreaddata(uint16_t regaddress,void *readval);
uint8_t Fingertech_sp0828readchipid(void);
void Fingertech_out24mclkpininit(void);
void Fingertech_output24mclk(void);
void Fingertech_sp0828VideotoRecSet(void);
void Fingertech_sp0828slaveconfig240320(void);
void Fingertech_SP0828SlaveDataOutputEnable(void);
void Fingertech_SP0828Intpininit(void);
#endif

