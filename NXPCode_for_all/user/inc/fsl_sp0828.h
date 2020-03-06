/*
 * The Clear BSD License
 * Copyright (c) 2017-2018, NXP Semiconductors, Inc.
 * All rights reserved.
 *
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 *  that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _FSL_SP0828_H_
#define _FSL_SP0828_H_

#include "fsl_common.h"
#include "fsl_sccb.h"
#include "fsl_camera_device.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief Register definitions for the SP0828.*/

#define SP0828_GAIN_REG 0x00U       /*!< Gain control gain setting */
#define SP0828_BLUE_REG 0x01U       /*!< Blue channel gain setting */
#define SP0828_RED_REG 0x02U        /*!< Red channel gain setting. */
#define SP0828_GREEN_REG 0x03U      /*!< Green channel gain setting */
#define SP0828_BAVG_REG 0x05U       /*!< B Average Level */
#define SP0828_GAVG_REG 0x06U       /*!< G Average Level */
#define SP0828_RAVG_REG 0x07U       /*!< R Average Level */
#define SP0828_AECH_REG 0x08U       /*!< Exposure Value - AEC MSBs */
#define SP0828_COM2_REG 0x09U       /*!< Common Control 2 */
#define SP0828_PID_REG 0x0AU        /*!< Product ID Number MSB */
#define SP0828_VER_REG 0x0BU        /*!< Product ID Number LSB */
#define SP0828_COM3_REG 0x0CU       /*!< Common Control 3 */
#define SP0828_COM4_REG 0x0DU       /*!< Common Control 4 */
#define SP0828_COM5_REG 0x0EU       /*!< Common Control 5 */
#define SP0828_COM6_REG 0x0FU       /*!< Common Control 6 */
#define SP0828_AEC_REG 0x10U        /*!< Exposure Value */
#define SP0828_CLKRC_REG 0x11U      /*!< Internal Clock */
#define SP0828_COM7_REG 0x12U       /*!< Common Control 7 */
#define SP0828_COM8_REG 0x13U       /*!< Common Control 8 */
#define SP0828_COM9_REG 0x14U       /*!< Common Control 9 */
#define SP0828_COM10_REG 0x15U      /*!< Common Control 10 */
#define SP0828_REG16_REG 0x16U      /*!< Register 16 */
#define SP0828_HSTART_REG 0x17U     /*!< Horizontal Frame (HREF column) Start 8 MSBs */
#define SP0828_HSIZE_REG 0x18U      /*!< Horizontal Sensor Size */
#define SP0828_VSTART_REG 0x19U     /*!< Vertical Frame (row) Start 8 MSBs */
#define SP0828_VSIZE_REG 0x1AU      /*!< Vertical Sensor Size */
#define SP0828_PSHFT_REG 0x1BU      /*!< Data format */
#define SP0828_MIDH_REG 0x1CU       /*!< Manufacturer ID Byte - High */
#define SP0828_MIDL_REG 0x1DU       /*!< Manufacturer ID Byte - Low  */
#define SP0828_LAEC_REG 0x1FU       /*!< Fine AEC Value */
#define SP0828_COM11_REG 0x20U      /*!< Common Control 11 */
#define SP0828_BDBASE_REG 0x22U     /*!< Banding Filter Minimum AEC Value */
#define SP0828_BDMSTEP_REG 0x23U    /*!< Banding Filter Maximum Step */
#define SP0828_AEW_REG 0x24U        /*!< AGC/AEC Stable Operating Region (Upper Limit) */
#define SP0828_AEB_REG 0x25U        /*!< AGC/AEC Stable Operating Region (Lower Limit) */
#define SP0828_VPT_REG 0x26U        /*!< AGC/AEC Fast Mode Operating Region */
#define SP0828_REG28_REG 0x28U      /*!< Register 28 */
#define SP0828_HOUTSIZE_REG 0x29U   /*!< Horizontal Data Output Size 8 MSBs */
#define SP0828_EXHCH_REG 0x2AU      /*!< Dummy Pixel Insert MSB */
#define SP0828_EXHCL_REG 0x2BU      /*!< Dummy Pixel Insert LSB */
#define SP0828_VOUTSIZE_REG 0x2CU   /*!< Vertical Data Output Size MSBs */
#define SP0828_ADVFL_REG 0x2DU      /*!< LSB of Insert Dummy Rows in Vertical Sync (1 bit equals 1 row) */
#define SP0828_ADVFH_REG 0x2EU      /*!< MSB of Insert Dummy Rows in Vertical Sync */
#define SP0828_YAVE_REG 0x2FU       /*!< Y/G Channel Average Value */
#define SP0828_LUMHTH_REG 0x30U     /*!< Histogram AEC/AGC Luminance High Level Threshold */
#define SP0828_LUMLTH_REG 0x31U     /*!< Histogram AEC/AGC Luminance Low Level Threshold */
#define SP0828_HREF_REG 0x32U       /*!< Image Start and Size Control */
#define SP0828_DM_LNL_REG 0x33U     /*!< Low 8 Bits of the Number of Dummy Rows */
#define SP0828_DM_LNH_REG 0x34U     /*!< High 8 Bits of the Number of Dummy Rows */
#define SP0828_ADOFF_B_REG 0x35U    /*!< AD Offset Compensation Value for B Channel */
#define SP0828_ADOFF_R_REG 0x36U    /*!< AD Offset Compensation Value for R Channel */
#define SP0828_ADOFF_GB_REG 0x37U   /*!< AD Offset Compensation Value for Gb Channel */
#define SP0828_ADOFF_GR_REG 0x38U   /*!< AD Offset Compensation Value for Gr Channel */
#define SP0828_OFF_B_REG 0x39U      /*!< B Channel Offset Compensation Value */
#define SP0828_OFF_R_REG 0x3AU      /*!< R Channel Offset Compensation Value */
#define SP0828_OFF_GB_REG 0x3BU     /*!< Gb Channel Offset Compensation Value */
#define SP0828_OFF_GR_REG 0x3CU     /*!< Gr Channel Offset Compensation Value */
#define SP0828_COM12_REG 0x3DU      /*!< Common Control 12 */
#define SP0828_COM13_REG 0x3EU      /*!< Common Control 13 */
#define SP0828_COM14_REG 0x3FU      /*!< Common Control 14 */
#define SP0828_COM16_REG 0x41U      /*!< Common Control 16 */
#define SP0828_TGT_B_REG 0x42U      /*!< BLC Blue Channel Target Value */
#define SP0828_TGT_R_REG 0x43U      /*!< BLC Red Channel Target Value */
#define SP0828_TGT_GB_REG 0x44U     /*!< BLC Gb Channel Target Value */
#define SP0828_TGT_GR_REG 0x45U     /*!< BLC Gr Channel Target Value */
#define SP0828_LC_CTR_REG 0x46U     /*!< Lens Correction Control */
#define SP0828_LC_XC_REG 0x47U      /*!< X Coordinate of Lens Correction Center Relative to Array Center */
#define SP0828_LC_YC_REG 0x48U      /*!< Y Coordinate of Lens Correction Center Relative to Array Center */
#define SP0828_LC_COEF_REG 0x49U    /*!< Lens Correction Coefficient */
#define SP0828_LC_RADI_REG 0x4AU    /*!< Lens Correction Radius */
#define SP0828_LC_COEFB_REG 0x4BU   /*!< Lens Correction B Channel Compensation Coefficient */
#define SP0828_LC_COEFR_REG 0x4CU   /*!< Lens Correction R Channel Compensation Coefficient */
#define SP0828_FIXGAIN_REG 0x4DU    /*!< Analog Fix Gain Amplifier */
#define SP0828_AREF1_REG 0x4FU      /*!< Sensor Reference Current Control */
#define SP0828_AREF6_REG 0x54U      /*!< Analog Reference Control */
#define SP0828_UFIX_REG 0x60U       /*!< U Channel Fixed Value Output */
#define SP0828_VFIX_REG 0x61U       /*!< V Channel Fixed Value Output */
#define SP0828_AWBB_BLK_REG 0x62U   /*!< AWB Option for Advanced AWBA */
#define SP0828_AWB_CTRL0_REG 0x63U  /*!< AWB Control Byte 0 */
#define SP0828_DSP_CTRL1_REG 0x64U  /*!< DSP Control Byte 1 */
#define SP0828_DSP_CTRL2_REG 0x65U  /*!< DSP Control Byte 2 */
#define SP0828_DSP_CTRL3_REG 0x66U  /*!< DSP Control Byte 3 */
#define SP0828_DSP_CTRL4_REG 0x67U  /*!< DSP Control Byte 4 */
#define SP0828_AWB_BIAS_REG 0x68U   /*!< AWB BLC Level Clip */
#define SP0828_AWB_CTRL1_REG 0x69U  /*!< AWB Control 1 */
#define SP0828_AWB_CTRL2_REG 0x6AU  /*!< AWB Control 2 */
#define SP0828_AWB_CTRL3_REG 0x6BU  /*!< AWB Control 3 */
#define SP0828_AWB_CTRL4_REG 0x6CU  /*!< AWB Control 4 */
#define SP0828_AWB_CTRL5_REG 0x6DU  /*!< AWB Control 5 */
#define SP0828_AWB_CTRL6_REG 0x6EU  /*!< AWB Control 6 */
#define SP0828_AWB_CTRL7_REG 0x6FU  /*!< AWB Control 7 */
#define SP0828_AWB_CTRL8_REG 0x70U  /*!< AWB Control 8 */
#define SP0828_AWB_CTRL9_REG 0x71U  /*!< AWB Control 9 */
#define SP0828_AWB_CTRL10_REG 0x72U /*!< AWB Control 10 */
#define SP0828_AWB_CTRL11_REG 0x73U /*!< AWB Control 11 */
#define SP0828_AWB_CTRL12_REG 0x74U /*!< AWB Control 12 */
#define SP0828_AWB_CTRL13_REG 0x75U /*!< AWB Control 13 */
#define SP0828_AWB_CTRL14_REG 0x76U /*!< AWB Control 14 */
#define SP0828_AWB_CTRL15_REG 0x77U /*!< AWB Control 15 */
#define SP0828_AWB_CTRL16_REG 0x78U /*!< AWB Control 16 */
#define SP0828_AWB_CTRL17_REG 0x79U /*!< AWB Control 17 */
#define SP0828_AWB_CTRL18_REG 0x7AU /*!< AWB Control 18 */
#define SP0828_AWB_CTRL19_REG 0x7BU /*!< AWB R Gain Range */
#define SP0828_AWB_CTRL20_REG 0x7CU /*!< AWB G Gain Range */
#define SP0828_AWB_CTRL21_REG 0x7DU /*!< AWB B Gain Range */
#define SP0828_GAM1_REG 0x7EU       /*!< Gamma Curve 1st Segment Input End Point  0x04 Output Value */
#define SP0828_GAM2_REG 0x7FU       /*!< Gamma Curve 2nd Segment Input End Point  0x08 Output Value */
#define SP0828_GAM3_REG 0x80U       /*!< Gamma Curve 3rd Segment Input End Point  0x10 Output Value */
#define SP0828_GAM4_REG 0x81U       /*!< Gamma Curve 4th Segment Input End Point  0x20 Output Value */
#define SP0828_GAM5_REG 0x82U       /*!< Gamma Curve 5th Segment Input End Point  0x28 Output Value */
#define SP0828_GAM6_REG 0x83U       /*!< Gamma Curve 6th Segment Input End Point  0x30 Output Value */
#define SP0828_GAM7_REG 0x84U       /*!< Gamma Curve 7th Segment Input End Point  0x38 Output Value */
#define SP0828_GAM8_REG 0x85U       /*!< Gamma Curve 8th Segment Input End Point  0x40 Output Value */
#define SP0828_GAM9_REG 0x86U       /*!< Gamma Curve 9th Segment Input End Point  0x48 Output Value */
#define SP0828_GAM10_REG 0x87U      /*!< Gamma Curve 10th Segment Input End Point 0x50 Output Value */
#define SP0828_GAM11_REG 0x88U      /*!< Gamma Curve 11th Segment Input End Point 0x60 Output Value */
#define SP0828_GAM12_REG 0x89U      /*!< Gamma Curve 12th Segment Input End Point 0x70 Output Value */
#define SP0828_GAM13_REG 0x8AU      /*!< Gamma Curve 13th Segment Input End Point 0x90 Output Value */
#define SP0828_GAM14_REG 0x8BU      /*!< Gamma Curve 14th Segment Input End Point 0xB0 Output Value */
#define SP0828_GAM15_REG 0x8CU      /*!< Gamma Curve 15th Segment Input End Point 0xD0 Output Value */
#define SP0828_SLOP_REG 0x8DU       /*!< Gamma Curve Highest Segment Slope */
#define SP0828_DNSTH_REG 0x8EU      /*!< De-noise Threshold */
#define SP0828_EDGE0_REG 0x8FU      /*!< Sharpness (Edge Enhancement) Control 0 */
#define SP0828_EDGE1_REG 0x90U      /*!< Sharpness (Edge Enhancement) Control 1 */
#define SP0828_DNSOFF_REG 0x91U     /*!< Lower Limit of De-noise Threshold - effective in auto mode only */
#define SP0828_EDGE2_REG 0x92U      /*!< Sharpness (Edge Enhancement) Strength Upper Limit */
#define SP0828_EDGE3_REG 0x93U      /*!< Sharpness (Edge Enhancement) Strength Lower Limit */
#define SP0828_MTX1_REG 0x94U       /*!< Matrix Coefficient 1 */
#define SP0828_MTX2_REG 0x95U       /*!< Matrix Coefficient 2 */
#define SP0828_MTX3_REG 0x96U       /*!< Matrix Coefficient 3 */
#define SP0828_MTX4_REG 0x97U       /*!< Matrix Coefficient 4 */
#define SP0828_MTX5_REG 0x98U       /*!< Matrix Coefficient 5 */
#define SP0828_MTX6_REG 0x99U       /*!< Matrix Coefficient 6 */
#define SP0828_MTX_CTRL_REG 0x9AU   /*!< Matrix Control */
#define SP0828_BRIGHT_REG 0x9BU     /*!< Brightness */
#define SP0828_CNST_REG 0x9CU       /*!< Contrast */
#define SP0828_UVADJ0_REG 0x9EU     /*!< Auto UV Adjust Control 0 */
#define SP0828_UVADJ1_REG 0x9FU     /*!< Auto UV Adjust Control 1 */
#define SP0828_SCAL0_REG 0xA0U      /*!< DCW Ratio Control */
#define SP0828_SCAL1_REG 0xA1U      /*!< Horizontal Zoom Out Control */
#define SP0828_SCAL2_REG 0xA2U      /*!< Vertical Zoom Out Control */
#define SP0828_SDE_REG 0xA6U        /*!< Special Digital Effect (SDE) Control */
#define SP0828_USAT_REG 0xA7U       /*!< U Component Saturation Gain */
#define SP0828_VSAT_REG 0xA8U       /*!< V Component Saturation Gain */
#define SP0828_HUECOS_REG 0xA9U     /*!< Cosine value x 0x80 */
#define SP0828_HUESIN_REG 0xAAU     /*!< |Sine value| x 0x80 */
#define SP0828_SIGN_REG 0xABU       /*!< Sign Bit for Hue and Brightness */
#define SP0828_DSPAUTO_REG 0xACU    /*!< DSP Auto Function ON/OFF Control */

#define SP0828_COM10_VSYNC_NEG_MASK (1U << 1U)
#define SP0828_COM10_HREF_REVERSE_MASK (1U << 3U)
#define SP0828_COM10_PCLK_REVERSE_MASK (1U << 4U)
#define SP0828_COM10_PCLK_OUT_MASK (1U << 5U)
#define SP0828_COM10_DATA_NEG_MASK (1U << 7U)

/*!
 * @brief SP0828 resource.
 *
 * Before initialize the SP0828, the resource must be initialized that the SCCB
 * I2C could start to work.
 */
typedef struct _SP0828_resource
{
    void (*pullResetPin)(bool pullUp);      /*!< Function to pull reset pin high or low. */
    void (*pullPowerDownPin)(bool pullUp);  /*!< Function to pull the power down pin high or low. */
    uint32_t inputClockFreq_Hz;             /*!< Input clock frequency. */
} SP0828_resource_t;

/*! @brief SP0828 operation functions. */
extern const camera_device_operations_t SP0828_ops;

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(__cplusplus)
}
#endif

#endif /* _FSL_SP0828_H_ */
