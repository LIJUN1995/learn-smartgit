#ifndef _NAVIGATION_H
#define _NAVIGATION_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        NAVI_SUCCESS = 0,
        NAVI_FAILED = 0X80,
        NAVI_BAD_PARAM
    } NAVI_UNUSUAL_U;

    typedef enum
    {
        NAVI_NOT_FINGER = 0,
        NAVI_NOT_MOVE = 0X80,
        NAVI_MOVE_BAD_PARAM,
        NAVI_IS_FINGER
    } NAVI_MOVE_FLAG; //finger move flag

    //�洢����ͼ������ͼ
    typedef struct
    {
        uint16_t Full_B[SENSOR_COL * SENSOR_ROW];   //�洢ȫ�ɼ�����ͼ��12bit
        uint16_t Sample_B[SENSOR_COL * SENSOR_ROW]; //�洢��������ͼ, 12bit
        int32_t nFull_B_MeanBase;
        int32_t nSample_B_MeanBase;
        int32_t nFull_BkFFTEnergy;     //����ͼfft����ֵ
        int32_t nFull_BkFFTdiffEnergy; //��ָͼ�뱳��ͼfft����ֵ��
        int32_t nSample_BkFFTEnergy;
        int32_t nSample_BkFFTdiffEnergy;
        int32_t nNAV_IMAGE_GAP;
        int32_t nConvergenceFlag; //������־λ
        int32_t label[3];         //label[0]:�����ݷ������ꣻlabel[1]:���ĺ᷽�����ꣻlabel[2]����ǰͼ���Ƿ�����ָ��
    } NavigationStruct;

    NAVI_UNUSUAL_U NavigationInit(uint16_t *buffer, NavigationStruct *pNaviStr, int32_t *NaviType, int32_t nNAV_IMAGE_GAP); // ������

    NAVI_UNUSUAL_U update_nav(NavigationStruct *pNaviStr, uint16_t *img, int32_t img_height, int32_t img_width);

    NAVI_MOVE_FLAG isNaviFingerDown(uint16_t *buffer, NavigationStruct *pNaviStr, int32_t *NaviType);
    NAVI_MOVE_FLAG isNaviFingerUp(uint16_t *buffer, NavigationStruct *pNaviStr, int32_t *NaviType);
    NAVI_MOVE_FLAG isNaviCaliFingerDown(uint16_t *buffer, NavigationStruct *pNaviStr, int32_t *NaviType);
    NAVI_MOVE_FLAG FFT_ENERGY_NAVI(NavigationStruct *pNaviStr, uint16_t *buffer, int32_t *NaviType, int32_t *SampleType, int32_t nAmpThre);
    NAVI_MOVE_FLAG isUpdateImage(uint16_t *buffer, NavigationStruct *pNaviStr, int32_t *NaviType);
#ifdef __cplusplus
}
#endif

#endif
