#include "FingerTech_common.h"
#include "FingerTech_spiConfig.h"

int main(void)
{
	FingerTech_BspInit();
	Fingertech_SpiConfig();
	
	while(1){	
		if(flag == READ_IMAGE){
			switch(slaveRxData[1]){
				case EXPOSURE_TIME:{
					Cdfinger_ChangeExposureTime();
				}
				break;
				
				case EXPOSURE_GAIN:{
					FingerTech_change_exposureGain();
				}
				break;
				
				case MV_IMG:{
					Cdfinger_MoveImage();
				}
				break;
				
				case ENROLL:{
					Cdfinger_EnrollPrepareTransfer();
				}
				break;
				
				case MULTI_FRAME:{
					Cdfinger_RetryRecPrepareTransfer();
				}
				break;
				
				case SINGLE_FRAME:{
					Cdfinger_RecPrepareTransfer();
				}
				break;
				
				case GET_ATTRIBUTE:{
					Cdfinger_AttributeTransfer();
				}
				break;
				
				case SET_COORDINATES:{
					Cdfinger_SetCoordinates();
				}
				break;
				
				default:
					DEBUG("failed order\n");
			}
			
		//  LPUART_WriteBlocking(LPUART1,(uint8_t*)s_frameBuffer,WIDTH*HEIGHT);
		}
	}
}
