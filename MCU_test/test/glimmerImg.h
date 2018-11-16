/******************** (C) COPYRIGHT 2015 GLIMMER
*********************************
* File Name          : glimmerImg.h
* Author               : wl
* Version              : 1.0
* Date                  : 2015.1.9
*******************************************************************************/

#ifndef GR_IMAGE_H
#define GR_IMAGE_H

#include <stdint.h>

typedef struct _glimmerImageformate {
  uint8_t *buffer;
  int16_t cols;
  int16_t rows;
} glimmerImagefomate;

typedef struct _glimmerImagePattern {
  int16_t width;
  int16_t height;
  int8_t bits;
  int8_t channels;
  int8_t format;
} glimmerImagePattern;

typedef struct _glimmerImage {
  uint8_t *buffer;
  glimmerImagePattern layout;
  uint32_t ability;
  uint16_t CT;
  uint32_t nValid;
  uint32_t nRatio;
  uint32_t nImagLen;
} glimmerImage;

typedef struct _glimmerQuaScore {
  uint32_t overlap;
  uint32_t quality;
  uint32_t nMaxNum;
  uint32_t nValidArea;
  uint32_t nImageLen;
  uint32_t nRaio;
} glimmerQuaScore;

#endif
