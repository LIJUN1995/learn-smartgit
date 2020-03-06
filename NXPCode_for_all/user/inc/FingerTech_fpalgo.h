#ifndef _FINGERTECH_FPALGO_H_
#define _FINGERTECH_FPALGO_H_

#include "main.h"
#define ALGO_MEMPOOL_LEN (3*1024*1024)
#define THR_SELECTBMP (1000)
#define IS_FLOTING       0x01
#define IS_PIXEL_CANCEL  0x00
#define IS_COATING 0x00

//支持的手指数目
#define FINGTECT_SUPPORTFINGERNUM   10
//接收命令码的长度
#define FINGTECH_RECEIVELEN         8
#define FINGERTECH_RESULTLEN        3
//注册次数
#define FINGERTECH_ENROLL           8

typedef enum
{
    FP_LIB_OK,                            // 0
    FP_LIB_CAPTURE_DONE_FFT, 
    FP_LIB_CAPTURE_DONE,                  //
    FP_LIB_FINGER_LOST,                   // 
    FP_LIB_TOO_FAST,                //
    FP_LIB_TOO_SLOW,                // 
    FP_LIB_LOW_QUALITY,

    FP_LIB_ERROR_GENERAL,                 //
    FP_LIB_ERROR_SENSOR,                  // 
    FP_LIB_ERROR_MEMORY,                  //
    FP_LIB_ERROR_PARAMETER,               // 
    FP_LIB_ERROR_EROLL_EXCEED_MAX_FINGERPIRNTS,
    FP_LIB_ERROR_EROLL_NOT_COMPLETED,
    FP_LIB_ERROR_EROLL_FAIL,
} fp_lib_return_t;


typedef enum
{
    FP_LIB_ENROLL_SUCCESS,
    FP_LIB_ENROLL_FAIL_NONE,
	FP_LIB_ENROLL_FAIL_DUPLICATE_AERA,
    FP_LIB_ENROLL_FAIL_LOW_QUALITY,
    FP_LIB_ENROLL_FAIL_LOW_COVERAGE,
    FP_LIB_ENROLL_ERROR_FAIL,
    FP_LIB_ENROLL_ERROR_START_FAIL,
    FP_LIB_ENROLL_ERROR_NOSPACE
} fp_lib_enroll_result_t;


typedef struct
{
    fp_lib_enroll_result_t result;
    int32_t progress;
    int32_t quality;
    int32_t coverage;
} fp_lib_enroll_data_t;


void FingerTech_varinit(void);
uint32_t Fingertech_InitAlgoUnit(void *g_pMemBaseAddr, int max_len);
int Fingertech_check_finger_down( void *image, int *is_finger );
uint32_t FingerTech_enroll_start_cfp(void);
int FingerTech_enroll_iterate_cfp( fp_lib_enroll_data_t *fp_enrol_data, void *image );
int FingerTech_fpauthenticate_iterate_cfp(void* image,GR_Template **pTemplate);
int FingerTech_fpauthenticate(void);
uint8_t FingerTech_fpenroll(void);


#endif

