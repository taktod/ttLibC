#include "../encoder.hpp"
#include <ttLibC/encoder/msAacEncoder.h>
#include <ttLibC/util/msGlobalUtil.h>
#include <ttLibC/util/beepUtil.h>
#include <ttLibC/container/flv.h>

using namespace std;

#ifdef __ENABLE_WIN32__
#define MSAAC(A, B) TEST_F(EncoderTest, MsAac##A){B();}
#else
#define MSAAC(A, B) TEST_F(EncoderTest, DISABLED_MsAac##A){}
#endif

MSAAC(EncodeTest, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();

  auto generator = ttLibC_BeepGenerator_make(PcmS16Type_littleEndian, 440, 44100, 1);
  ttLibC_PcmS16 *pcm = nullptr;
  auto encoder = ttLibC_MsAacEncoder_make(44100, 1, 96000);
  int counter = 0;
  for(int i = 0;i < 5;++ i) {
    pcm = ttLibC_BeepGenerator_makeBeepBySampleNum(generator, pcm, 1000);
    ttLibC_MsAacEncoder_encode(encoder, pcm, [](void *ptr, ttLibC_Aac *aac) {
      int *counter = reinterpret_cast<int *>(ptr);
      *counter += 1;
      return true;
    }, &counter);
  }
  ttLibC_PcmS16_close(&pcm);
  ttLibC_MsAacEncoder_close(&encoder);
  ttLibC_BeepGenerator_close(&generator);

  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
  EXPECT_GT(counter, 2);
});

MSAAC(FlvOutputTest, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();

  auto generator = ttLibC_BeepGenerator_make(PcmS16Type_littleEndian, 440, 44100, 1);
  auto encoder = ttLibC_MsAacEncoder_make(44100, 1, 96000);
  ttLibC_PcmS16 *pcm = nullptr;
  typedef struct {
    ttLibC_FlvWriter *writer;
    FILE *fp;
    int counter;
    uint64_t pts;
  } holder_t;
  holder_t holder;
  holder.writer = ttLibC_FlvWriter_make(frameType_unknown, frameType_aac);
  holder.fp = fopen("msaac_rec.flv", "wb");
  holder.counter = 0;
  if(holder.fp) {
    for(int i = 0;i < 50;++ i) {
      pcm = ttLibC_BeepGenerator_makeBeepBySampleNum(generator, pcm, 1000);
      ttLibC_MsAacEncoder_encode(encoder, pcm, [](void *ptr, ttLibC_Aac *aac) {
        holder_t *holder = reinterpret_cast<holder_t *>(ptr);
        holder->counter ++;
        aac->inherit_super.inherit_super.id = 8;
        aac->inherit_super.inherit_super.timebase = aac->inherit_super.sample_rate;
        aac->inherit_super.inherit_super.pts = holder->pts;
        holder->pts += aac->inherit_super.sample_num;
        return ttLibC_FlvWriter_write(holder->writer, (ttLibC_Frame *)aac, [](void *ptr, void *data, size_t data_size){
          holder_t *holder = reinterpret_cast<holder_t *>(ptr);
          fwrite(data, 1, data_size, holder->fp);
          return true;
        }, ptr);
      }, &holder);
    }
    fclose(holder.fp);
  }
  ttLibC_PcmS16_close(&pcm);
  ttLibC_FlvWriter_close(&holder.writer);
  ttLibC_MsAacEncoder_close(&encoder);
  ttLibC_BeepGenerator_close(&generator);

  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
  EXPECT_GT(holder.counter, 2);
});

#undef MSAAC
