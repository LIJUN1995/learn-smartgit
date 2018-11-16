/******************** (C) COPYRIGHT 2015 GLIMMER
*********************************
* File Name          : glimmerAL.h
* Author               : wl
* Version              : 1.0
* Date                  : 2015.1.9
*******************************************************************************/

#ifndef GR_AL_H
#define GR_AL_H

#include "glimmerImg.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _GR_Template {
  void *pFingerTemplate;
  int32_t imgLen;
} GR_Template;

typedef struct _G_TEMPLATE { void *pFingerFeature; } G_TEMPLATE;

typedef struct _G_MATCH_Data { void *pMatchData; } G_MATCH_Data;

typedef struct _G_LEARN_Data {
  G_TEMPLATE *pFpFtr;
  G_MATCH_Data *pMatchInfor;
} G_LEARN_Data;

typedef struct _GR_Template_Data {
  void *buffer;
  int32_t imgLen;
} GR_Template_Data;

typedef struct _GR_REGISTER_DATA {
  GR_Template *pFingerTemplate;
  int32_t imgLen;
} GR_REGISTER_DATA;

typedef struct _REGISTER_STANDARD {
  GR_REGISTER_DATA *pStandardData;
  G_TEMPLATE *pFpFtr;
  int16_t nMaxNum;
  int16_t nUsedNum;
  int32_t nSchedule;
  int32_t imgLen;
  int32_t nTotalCt;
  int32_t nIndependentCt;
} REGISTER_STANDARD;

typedef enum {
  GR_SUCCESS = 0,
  GR_FAILED = 0X80,
  GR_LOW_QUALITY,  // image low quality
  GR_LOW_COVER,    // image low cover
  GR_HIGH_OVERLAY, // image high overlay
  GR_ALREADY_REG,  //注册已满
  GR_BAD_PARAM,
  GR_NO_MEMORY,
  GR_NULL_POINT,
  GR_NON_LEARN,
  GR_CONTINUE_SUCCESS,
  GR_CONTINUE_FAILED
} GR_UNUSUAL_U;

typedef enum {
  GR_NOT_FINGER = 0,
  GR_NOT_MOVE = 0X80,
  GR_MOVE_BAD_PARAM,
  GR_MOVE_FAST,
  GR_MOVE_LEFT,
  GR_MOVE_RIGHT,
  GR_MOVE_UP,
  GR_MOVE_DOWN,
  GR_IS_FINGER,
  GR_IS_FFT_FINGER
} GR_MOVE_FLAG; // finger move flag

REGISTER_STANDARD *beginRegister();

G_LEARN_Data *beginRecognition();

//正常注册接口
GR_UNUSUAL_U registerImage(REGISTER_STANDARD *pStandard, glimmerImage *img,
                           glimmerQuaScore *quaImg, int32_t *nUpdate,
                           int32_t *nOL, int32_t nNotMesFrms,
                           int32_t overlapThre);
GR_UNUSUAL_U registerImage_LoadPreImage(
    REGISTER_STANDARD *pStandard, glimmerImage *img, glimmerQuaScore *quaImg,
    int32_t *nUpdate, int32_t *nOL, int32_t nNotMesFrms, int32_t overlapThre);
//华为注册接口
GR_UNUSUAL_U registerImage_HW(REGISTER_STANDARD *pStandard, glimmerImage *img,
                              glimmerQuaScore *quaImg, int32_t *nUpdate,
                              int32_t *nOL, int32_t nNotMesFrms1,
                              int32_t nNotMesFrms2, int32_t nContinue,
                              int32_t nInterval, int32_t overlapThre);

//华为跑库注册接口
GR_UNUSUAL_U registerImage_HW_LoadPreImage(
    REGISTER_STANDARD *pStandard, glimmerImage *img, glimmerQuaScore *quaImg,
    int32_t *nUpdate, int32_t *nOL, int32_t nNotMesFrms1, int32_t nNotMesFrms2,
    int32_t nContinue, int32_t nInterval, int32_t overlapThre);

GR_UNUSUAL_U rec_registerImage(G_TEMPLATE *pFpFtr, GR_Template **candidates,
                               uint32_t candidateNum, int32_t *whichOneTemplate,
                               int32_t *whichOnePatch, int32_t *score,
                               int32_t *isupdate, int32_t nRegCt);
GR_UNUSUAL_U registerGainTemplate(REGISTER_STANDARD *Standard,
                                  GR_Template **tpl);

GR_UNUSUAL_U EndRegister(REGISTER_STANDARD **Standard,
                         GR_Template **pTemplateData);
GR_UNUSUAL_U EndRecognition(G_LEARN_Data **p_gLearnData);

GR_UNUSUAL_U glimmer_Rec_ComputeFtr(glimmerImage *img,
                                    G_LEARN_Data *p_gLearnData);
GR_UNUSUAL_U glimmer_Reg_ComputeFtr(glimmerImage *img,
                                    REGISTER_STANDARD *pStandard);

GR_UNUSUAL_U glimmerRecognition(G_TEMPLATE *pFpFtr, G_MATCH_Data *pMatchInfor,
                                GR_Template **candidates, uint32_t candidateNum,
                                int32_t *whichOneTemplate,
                                int32_t *whichOnePatch, int32_t *score,
                                int32_t *isupdate);
GR_UNUSUAL_U glimmerRecognition_NotMatch(
    G_TEMPLATE *pFpFtr, G_MATCH_Data *pMatchInfor, GR_Template **candidates,
    uint32_t candidateNum, int32_t *whichOneTemplate, int32_t *whichOnePatch,
    int32_t *score, int32_t *isupdate, int32_t *nNotMatch);

GR_UNUSUAL_U glimmerLearn(glimmerImage *img, G_TEMPLATE *pFpFtr,
                          G_MATCH_Data *pMatchInfor, GR_Template **candidates,
                          int32_t whichOneTemplate, int32_t whichOnePatch,
                          int32_t isupdate);

uint32_t GainReduceTemplateDataedLen(GR_Template *pTemplateData);

GR_UNUSUAL_U ReduceTemplateData(GR_Template *pTemplateData, uint8_t *dst);

GR_UNUSUAL_U IncreaseTemplateData(uint8_t *Source, uint32_t length,
                                  GR_Template **pTemplateData);

void IndefyImage(GR_Template *pTemplateData, uint8_t *dst);
void RegsiterImage(GR_Template *pTemplateData, uint8_t *source);

GR_UNUSUAL_U FreeTemplate(GR_Template **pTemplateData);

//获得图像质量
// 1、正常获得图像质量接口
GR_UNUSUAL_U GainQua(glimmerImage *img, uint8_t *pdiffImg, int32_t *nQua,
                     int32_t *nValidArea);
GR_UNUSUAL_U GainValidAreage(int16_t *pbk_img, uint16_t *pCurImg,
                             int32_t *nValidArea, int32_t nRow, int32_t nCol);
// 2、跑库获得预处理图像质量接口
GR_UNUSUAL_U GainQua_LoadPreImage(glimmerImage *img, int32_t *nQua,
                                  int32_t *nValidArea);
#ifdef __cplusplus
}
#endif

#endif
