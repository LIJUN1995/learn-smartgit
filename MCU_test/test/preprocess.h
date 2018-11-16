#ifndef PREPROCESS_H
#define PREPROCESS_H

#include "glimmerAl.h"
#include "glimmerImg.h"
#ifdef __cplusplus
extern "C" {
#endif

#define RAWDATA_THRESHOLD 200 // threshold for the raw data
#define EIGHT_ORI_Q 0         // eight ori quantization
#if EIGHT_ORI_Q
#define QUAN_WIN_HALF_WIDTH 6 // half of quantization window width, default is 6
#else
#define QUAN_WIN_HALF_WIDTH 5 // half of quantization window width, default is 5
#endif
#define SMOOTH_QUAN_WIN_HALF_WIDTH                                             \
  1 // half of the window with of smoothing quantization, default is 1.
#define IS_INVERT 1 // whether reverse
#define SITO_BASELINE 5000
#define PARRLLEL_THRESHOLD 140
#define WHITE_DIFF_THRESHOLD 80
#define WHITE_ENABLE 1

#define ENERGY_THR1 2000
#define ENERGY_THR2 4000
#define SIMSCORE_THR1 250
#define ENERGY_RATIO1 205 // 256*0.8
#define ENERGY_RATIO2 384 // 256*1.5
#define SIMSCORE_THR2 217
#define SIMSCORE_THR3 200

#define SITO_COL_FRAMES 10

#define SENSOR_COL 120
#define SENSOR_ROW 120

typedef struct {
  uint32_t frameNum0;
  uint32_t frameNum1;
  uint32_t seleteIndex; // Index 0: no sito; 1: sito
  uint8_t dataBmp[SENSOR_COL *
                  SENSOR_ROW]; // Bmp after calibration and quantization
  uint16_t sitoBmp[SENSOR_COL * SENSOR_ROW];
  int16_t Kr[SENSOR_COL * SENSOR_ROW]; // Auto calibration parameters
  int16_t B[SENSOR_COL * SENSOR_ROW];  // Baseline no touch
  int16_t extraGSK[SENSOR_ROW * SENSOR_COL];
  int16_t framesData[SENSOR_ROW * SENSOR_COL];
  uint16_t caliRes[SENSOR_COL * SENSOR_ROW]; // calibration data
} calibrationPara;

GR_UNUSUAL_U preProcess(uint16_t *buffer, int32_t bufferLen,
                        calibrationPara *caliPara, int32_t flagChipInfo,
                        int32_t *calType, int32_t thrSelectBmp);

GR_UNUSUAL_U saveCalibrationPara(calibrationPara *caliPara, uint8_t *dst);
GR_UNUSUAL_U readCalibrationPara(calibrationPara *caliPara, uint8_t *dst);

#ifdef __cplusplus
}
#endif

#endif
