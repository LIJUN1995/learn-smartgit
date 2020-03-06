#ifndef __FINGERTECH_LED_H
#define __FINGERTECH_LED_H
#include "fsl_common.h"
#include "fsl_iomuxc.h"
#include "main.h"

extern uint8_t slaveRxData[32];
extern uint8_t slaveTxData[32];

#ifdef P1801
extern uint16_t DeNoise_Rx[TRANSFER_SIZE];
extern uint16_t DeNoise_Tx[TRANSFER_SIZE];
extern uint16_t s_frameBuffer[TRANSFER_SIZE];
#endif 

#ifdef P1701
extern uint8_t DeNoise_Rx[TRANSFER_SIZE_O];
extern uint8_t DeNoise_Tx[TRANSFER_SIZE_O];
extern uint8_t s_frameBuffer[TRANSFER_SIZE_O];
#endif 

void FingerTech_Delayus(uint32_t nus);
void FingerTech_Delayms(uint16_t nms);
void FingerTech_LedOn(void);
void FingerTech_LedOff(void);
void FingerTech_LedInit(void);
void FingerTech_BspInit(void);
void FingerTech_BspUartInit(void);
void Cdfinger_SetCoordinates(void);
void FingerTech_FpIntPinInit(void);
void FingerTech_FpIntEnable(void);
void FingerTech_FpIntDisable(void);
void Cdfinger_RecPrepareTransfer(void);
void Cdfinger_AttributeTransfer(void);
void Cdfinger_RetryRecPrepareTransfer(void);
void Cdfinger_EnrollPrepareTransfer(void);
void Cdfinger_GetImage(uint8_t num);
void Cdfinger_MoveImage(void);
void Cdfinger_ChangeExposureTime(void);
#endif
