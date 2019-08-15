/**
 * @file   msH264Encoder.cpp
 * @brief  windows native h264 encoder.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2017/05/13
 */

#ifndef TTLIBC_ENCODER_MSH264ENCODER_H_
#define TTLIBC_ENCODER_MSH264ENCODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../ttLibC_predef.h"
#include "../frame/video/h264.h"
#include "../frame/video/yuv420.h"

typedef struct ttLibC_Encoder_MsH264Encoder {
  uint32_t width;
  uint32_t height;
  uint32_t bitrate;
} ttLibC_Encoder_MsH264Encoder;

typedef ttLibC_Encoder_MsH264Encoder ttLibC_MsH264Encoder;

typedef enum ttLibC_MsH264Encoder_rateType {
  MsH264EncoderRateType_CBR,
  MsH264EncoderRateType_ConstraintVBR,
  MsH264EncoderRateType_VBR,
  MsH264EncoderRateType_CQP
} ttLibC_MsH264Encoder_rateType;

typedef enum ttLibC_MsH264Encoder_Profile {
  MsH264EncoderProfile_Base,
  MsH264EncoderProfile_Main,
  MsH264EncoderProfile_High
} ttLibC_MsH264Encoder_Profile;

typedef struct ttLibC_MsH264Encoder_param {
  uint32_t                      width;
  uint32_t                      height;
  uint32_t                      bitrate;
  uint32_t                      fps;
  uint32_t                      maxBitrate;
  ttLibC_MsH264Encoder_rateType rateType;
  uint32_t                      GOP;
  uint32_t                      maxQp;
  uint32_t                      minQp;
  bool                          useLowLatency;
  uint32_t                      bufferSize;
  uint32_t                      bFrameCount;
  bool                          useCabac;
  ttLibC_MsH264Encoder_Profile  profile;
  int32_t                       level;
} ttLibC_MsH264Encoder_param;

typedef bool (*ttLibC_MsH264EncodeFunc)(void *ptr, ttLibC_H264 *h264);
typedef bool (*ttLibC_MsH264EncoderNameFunc)(void *ptr, const wchar_t *name);

ttLibC_MsH264Encoder TT_ATTRIBUTE_API *ttLibC_MsH264Encoder_make(
  const wchar_t *target,
  uint32_t width,
  uint32_t height,
  uint32_t bitrate);

ttLibC_MsH264Encoder TT_ATTRIBUTE_API *ttLibC_MsH264Encoder_makeWithParam(
  const wchar_t *target,
  ttLibC_MsH264Encoder_param *param);

bool TT_ATTRIBUTE_API ttLibC_MsH264Encoder_listEncoders(
  ttLibC_MsH264EncoderNameFunc callback,
  void *ptr);

bool TT_ATTRIBUTE_API ttLibC_MsH264Encoder_encode(
  ttLibC_MsH264Encoder *encoder,
  ttLibC_Yuv420 *frame,
  ttLibC_MsH264EncodeFunc callback,
  void *ptr);

void TT_ATTRIBUTE_API ttLibC_MsH264Encoder_close(ttLibC_MsH264Encoder **encoder);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_ENCODER_MSH264ENCODER_H_ */
