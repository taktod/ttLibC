#include "../decoder.hpp"
#include <ttLibC/util/hexUtil.h>
#include <ttLibC/decoder/audioConverterDecoder.h>
#include <ttLibC/frame/audio/aac2.h>

using namespace std;

#ifdef __ENABLE_APPLE__
#define AUDIOCONVERTER(A, B) TEST_F(DecoderTest, AudioConverter##A){B();}
#else
#define AUDIOCONVERTER(A, B) TEST_F(DecoderTest, AudioConverter##A){}
#endif

AUDIOCONVERTER(DecodeTest, [this](){
  auto decoder = ttLibC_AcDecoder_make(44100, 2, frameType_aac);
  // これをdecodeしてみることにしよう。
  ttLibC_Aac2 *aac = nullptr;
  ttLibC_Aac2 *a = nullptr;
  char data[256];
  size_t dataSize;
  int counter = 0;
  dataSize = ttLibC_HexUtil_makeBuffer("12 10 ", data, 256);
  a = ttLibC_Aac2_getFrame(aac, data, dataSize, true, 0, 1000);
  if(a != nullptr) {
    aac = a;
  }
  ttLibC_AcDecoder_decode(decoder, (ttLibC_Audio *)aac, [](void *ptr, ttLibC_PcmS16 *pcm){
    int *counter = reinterpret_cast<int *>(ptr);
    (*counter) ++;
    puts("aa");
    return true;
  }, &counter);
  dataSize = ttLibC_HexUtil_makeBuffer("21 00 49 90 02 19 00 23 80 ", data, 256);
  a = ttLibC_Aac2_getFrame(aac, data, dataSize, true, 0, 1000);
  if(a != nullptr) {
    aac = a;
  }
  ttLibC_AcDecoder_decode(decoder, (ttLibC_Audio *)aac, [](void *ptr, ttLibC_PcmS16 *pcm){
    int *counter = reinterpret_cast<int *>(ptr);
    (*counter) ++;
    puts("bb");
    return true;
  }, &counter);
  dataSize = ttLibC_HexUtil_makeBuffer("21 00 49 90 02 19 00 23 80 ", data, 256);
  a = ttLibC_Aac2_getFrame(aac, data, dataSize, true, 23, 1000);
  if(a != nullptr) {
    aac = a;
  }
  ttLibC_AcDecoder_decode(decoder, (ttLibC_Audio *)aac, [](void *ptr, ttLibC_PcmS16 *pcm){
    int *counter = reinterpret_cast<int *>(ptr);
    (*counter) ++;
    puts("bb");
    return true;
  }, &counter);
  dataSize = ttLibC_HexUtil_makeBuffer("21 00 49 90 02 19 00 23 80 ", data, 256);
  a = ttLibC_Aac2_getFrame(aac, data, dataSize, true, 46, 1000);
  if(a != nullptr) {
    aac = a;
  }
  ttLibC_AcDecoder_decode(decoder, (ttLibC_Audio *)aac, [](void *ptr, ttLibC_PcmS16 *pcm){
    int *counter = reinterpret_cast<int *>(ptr);
    (*counter) ++;
    puts("bb");
    return true;
  }, &counter);
  dataSize = ttLibC_HexUtil_makeBuffer("21 00 49 90 02 19 00 23 80 ", data, 256);
  a = ttLibC_Aac2_getFrame(aac, data, dataSize, true, 70, 1000);
  if(a != nullptr) {
    aac = a;
  }
  ttLibC_AcDecoder_decode(decoder, (ttLibC_Audio *)aac, [](void *ptr, ttLibC_PcmS16 *pcm){
    int *counter = reinterpret_cast<int *>(ptr);
    (*counter) ++;
    puts("bb");
    return true;
  }, &counter);
  dataSize = ttLibC_HexUtil_makeBuffer("21 20 49 90 02 19 00 23 80 ", data, 256);
  a = ttLibC_Aac2_getFrame(aac, data, dataSize, true, 93, 1000);
  if(a != nullptr) {
    aac = a;
  }
  ttLibC_AcDecoder_decode(decoder, (ttLibC_Audio *)aac, [](void *ptr, ttLibC_PcmS16 *pcm){
    int *counter = reinterpret_cast<int *>(ptr);
    (*counter) ++;
    puts("bb");
    return true;
  }, &counter);
  dataSize = ttLibC_HexUtil_makeBuffer("21 41 FE D6 80 85 A0 23 80 ", data, 256);
  a = ttLibC_Aac2_getFrame(aac, data, dataSize, true, 116, 1000);
  if(a != nullptr) {
    aac = a;
  }
  ttLibC_AcDecoder_decode(decoder, (ttLibC_Audio *)aac, [](void *ptr, ttLibC_PcmS16 *pcm){
    int *counter = reinterpret_cast<int *>(ptr);
    (*counter) ++;
    puts("bb");
    return true;
  }, &counter);
/*
  "12 10 "
  "21 00 49 90 02 19 00 23 80 "
  "21 00 49 90 02 19 00 23 80 "
  "21 00 49 90 02 19 00 23 80 "
  "21 00 49 90 02 19 00 23 80 "
  "21 20 49 90 02 19 00 23 80 "
  "21 41 FE D6 80 85 A0 23 80 "*/
  ttLibC_Aac2_close(&aac);
  ttLibC_AcDecoder_close(&decoder);
  ASSERT_GT(counter, 0);
});

#undef AUDIOCONVERTER
