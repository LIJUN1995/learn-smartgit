#include "main.h"

uint8_t receivecount = 0;
volatile bool CSI_intflag = true;
extern volatile bool s_frameDone ;
extern void CSI_DriverIRQHandler(void);

/****************************
功能: 设置中断组
说明: 
****************************/
void FingerTech_NVIC_SetGrouping(uint32_t group)
{
    NVIC_SetPriorityGrouping(group);
}

/****************************
功能: 设置中断优先级
说明: IRQn 中断号
      prep 抢占优先级
      subp 子优先级
****************************/
void FingerTech_NVIC_SetPriority(IRQn_Type IRQn,uint32_t prep,uint32_t subp)
{ 
	uint32_t prioritygroup = 0x00;
	prioritygroup=NVIC_GetPriorityGrouping();
	NVIC_SetPriority(IRQn,NVIC_EncodePriority(prioritygroup,prep,subp));
}

/****************************
功能: 指纹中断功能
说明: 
****************************/
/*
void GPIO3_Combined_16_31_IRQHandler(void)
{
    uint8_t clrcmd=0x14;
    uint8_t ret = 0;
    BaseType_t xHigherPriorityTaskWoken;
   // BaseType_t xResult;
    xHigherPriorityTaskWoken = pdFALSE;
    GPIO_PortClearInterruptFlags(GPIO3, 1U << 16U);
    FingerTech_Logd("i am fingerprint eint\r\n");
    FingerTech_SpiWRDate(&clrcmd,&ret,1);
   // xResult = xEventGroupSetBitsFromISR(gFingerTech_fingerEventGroup,FingerTech_intmask,&xHigherPriorityTaskWoken);
     xSemaphoreGiveFromISR(fpdevice.intflag,&xHigherPriorityTaskWoken);
     portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
*/

/****************************
功能: 触摸IC中断
说明: 
****************************/
#if 0
void GPIO5_Combined_0_15_IRQHandler(void)
{
   // BaseType_t xHigherPriorityTaskWoken;
   // xHigherPriorityTaskWoken = pdFALSE;
    GPIO_PortClearInterruptFlags(GPIO5, 1U << 0U);
    //xSemaphoreGiveFromISR(fpdevice.intflag,&xHigherPriorityTaskWoken);
}
#endif

/****************************
功能: 串口接收中断
说明: 
****************************/
#if 0
void LPUART1_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken;
    BaseType_t xResult;
    if ((kLPUART_RxDataRegFullFlag)&LPUART_GetStatusFlags(LPUART1))
    {
         gfingertech_receivedata[receivecount] = LPUART_ReadByte(LPUART1);
         receivecount ++;
         LPUART_ClearStatusFlags(LPUART1,kLPUART_RxDataRegFullFlag);
         if(FINGTECH_RECEIVELEN == receivecount)
         {
             receivecount = 0;
             xResult = xStreamBufferSendFromISR(xStreamBuffer,(void*)gfingertech_receivedata,FINGTECH_RECEIVELEN,&xHigherPriorityTaskWoken);
             if(xResult!=FINGTECH_RECEIVELEN)
             {
                 //FingerTech_Loge("xStreamBufferSendFromISR is faile");
             }
             portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
         }
         
    }
}
#endif


#if 0
void LCDIF_IRQHandler(void)
{
    uint32_t intStatus;
    intStatus = ELCDIF_GetInterruptStatus(LCDIF);
    ELCDIF_ClearInterruptStatus(LCDIF, intStatus);
    if (intStatus & kELCDIF_CurFrameDone)
    {
        s_frameDone = true;
    }
}
#endif
#if 0
void GPIO1_Combined_16_31_IRQHandler(void)
{
    //int i=0;
    //if(GPIO_GetPinsInterruptFlags(GPIO1)&(1<<26U))
    {
        /*
        Fingertech_sp0828iicwritedata(0x40,0x08);
        FingerTech_SpiReadDate(&sp0828imgbuf[0],10);
        //FingerTech_SpiWRDate(&sp0828imgtx[0],&sp0828imgbuf[10],10);
        printf("%d %d\r\n",sp0828imgbuf[0],sp0828imgbuf[1]);
        Fingertech_sp0828iicwritedata(0x40,0x00);
        */
        GPIO_PortClearInterruptFlags(GPIO1, 1U << 26U);
    }
}
#endif

/****************************
功能: 摄像头中断处理函数
说明: 
****************************/
void CSI_IRQHandler(void)
{
    uint32_t csisr = CSI->CSISR;
		
		DEBUG("entry IRQ");
		CSI_intflag = false;
	
    CSI->CSISR = csisr;   /* Clear the flags. */
    CSI_Stop(CSI);
}


void HardFault_Handler(void)
{
    //FingerTech_Loge("HardFault_Handler HardFault_Handler");
    while(1);
}
