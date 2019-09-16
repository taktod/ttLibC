#include "../resampler.hpp"
#include <ttLibC/resampler/msAudioResampler.h>
#include <ttLibC/resampler/msAudioResampler.hpp>
#include <ttLibC/encoder/msAacEncoder.h>
#include <ttLibC/util/msGlobalUtil.h>
#include <ttLibC/util/beepUtil.h>
#include <ttLibC/container/flv.h>

using namespace std;

#ifdef __ENABLE_WIN32__
#define MSAUDIO(A,B) TEST_F(ResamplerTest, MsAudio##A){B();}
#else
#define MSAUDIO(A,B) TEST_F(ResamplerTest, DISABLED_MsAudio##A){}
#endif

MSAUDIO(ResampleCxxTest, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  {
    ttLibC::MsAudioResampler resampler(
      frameType_pcmS16, PcmS16Type_littleEndian, 44100, 2,
      frameType_pcmS16, PcmS16Type_littleEndian, 48000, 2);
    auto generator = ttLibC_BeepGenerator_make(PcmS16Type_littleEndian, 440, 44100, 2);
    ttLibC_PcmS16 *pcm = nullptr;
    typedef struct {
      ttLibC_FlvWriter *writer;
      ttLibC_MsAacEncoder *encoder;
      FILE *fp;
      int counter;
    } holder_t;
    holder_t holder;
    holder.writer = ttLibC_FlvWriter_make(frameType_unknown, frameType_aac);
    holder.fp = fopen("msaudio_rec.flv", "wb");
    holder.encoder = ttLibC_MsAacEncoder_make(48000, 2, 96000);
    holder.counter = 0;
    if(holder.fp) {
      for(int i = 0;i < 10;++ i) {
        pcm = ttLibC_BeepGenerator_makeBeepBySampleNum(generator, pcm, 4410);
        resampler.resample((ttLibC_Audio *)pcm, [&](ttLibC_Audio *audio) {
          return ttLibC_MsAacEncoder_encode(holder.encoder, (ttLibC_PcmS16 *)audio, [](void *ptr, ttLibC_Aac *aac) {
            holder_t *holder = reinterpret_cast<holder_t *>(ptr);
            aac->inherit_super.inherit_super.id = 8;
            return ttLibC_FlvWriter_write(holder->writer, (ttLibC_Frame *)aac, [](void *ptr, void *data, size_t data_size){
              holder_t *holder = reinterpret_cast<holder_t *>(ptr);
              holder->counter ++;
              fwrite(data, 1, data_size, holder->fp);
              return true;
            }, ptr);
          }, &holder);
        });
      }
      fclose(holder.fp);
    }
    ttLibC_PcmS16_close(&pcm);
    ttLibC_MsAacEncoder_close(&holder.encoder);
    ttLibC_FlvWriter_close(&holder.writer);
    ttLibC_BeepGenerator_close(&generator);
    EXPECT_GT(holder.counter, 2);
  }
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSAUDIO(ResampleTest, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  {
    ttLibC_MsAudioResampler *resampler = ttLibC_MsAudioResampler_make(
      frameType_pcmS16, PcmS16Type_littleEndian, 44100, 2,
      frameType_pcmS16, PcmS16Type_littleEndian, 48000, 2);
    auto generator = ttLibC_BeepGenerator_make(PcmS16Type_littleEndian, 440, 44100, 2);
    ttLibC_PcmS16 *pcm = nullptr;
    typedef struct {
      ttLibC_FlvWriter *writer;
      ttLibC_MsAacEncoder *encoder;
      FILE *fp;
      int counter;
    } holder_t;
    holder_t holder;
    holder.writer = ttLibC_FlvWriter_make(frameType_unknown, frameType_aac);
    holder.fp = fopen("msaudio_rec2.flv", "wb");
    holder.encoder = ttLibC_MsAacEncoder_make(48000, 2, 96000);
    holder.counter = 0;
    if(holder.fp) {
      for(int i = 0;i < 10;++ i) {
        pcm = ttLibC_BeepGenerator_makeBeepBySampleNum(generator, pcm, 4410);
        ttLibC_MsAudioResampler_resample(resampler, (ttLibC_Audio *)pcm, [](void *ptr, ttLibC_Audio *audio){
          holder_t *holder = reinterpret_cast<holder_t *>(ptr);
          return ttLibC_MsAacEncoder_encode(holder->encoder, (ttLibC_PcmS16 *)audio, [](void *ptr, ttLibC_Aac *aac) {
            holder_t *holder = reinterpret_cast<holder_t *>(ptr);
            aac->inherit_super.inherit_super.id = 8;
            return ttLibC_FlvWriter_write(holder->writer, (ttLibC_Frame *)aac, [](void *ptr, void *data, size_t data_size){
              holder_t *holder = reinterpret_cast<holder_t *>(ptr);
              holder->counter ++;
              fwrite(data, 1, data_size, holder->fp);
              return true;
            }, ptr);
          }, ptr);
        }, &holder);
      }
      fclose(holder.fp);
      ttLibC_MsAudioResampler_close(&resampler);
    }
    ttLibC_PcmS16_close(&pcm);
    ttLibC_MsAacEncoder_close(&holder.encoder);
    ttLibC_FlvWriter_close(&holder.writer);
    ttLibC_BeepGenerator_close(&generator);
    EXPECT_GT(holder.counter, 2);
  }
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

#undef MSAUDIO