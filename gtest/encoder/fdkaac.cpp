#include "../encoder.hpp"
#include <ttLibC/util/beepUtil.h>
#include <ttLibC/encoder/fdkaacEncoder.h>
#include <ttLibC/container/flv.h>

using namespace std;

#ifdef __ENABLE_FDKAAC__
#define FDKAAC(A, B) TEST_F(EncoderTest, Fdkaac##A){B();}
#else
#define FDKAAC(A, B) TEST_F(EncoderTest, DISABLED_Fdkaac##A){}
#endif

FDKAAC(EncodeTest, [this](){
  auto generator = ttLibC_BeepGenerator_make(PcmS16Type_littleEndian, 440, 44100, 1);
  ttLibC_PcmS16 *pcm = nullptr;
  auto encoder = ttLibC_FdkaacEncoder_make("AOT_AAC_LC", 44100, 1, 96000);
  int counter = 0;
  for(int i = 0;i < 5;++ i) {
    pcm = ttLibC_BeepGenerator_makeBeepBySampleNum(generator, pcm, 1000);
    ttLibC_FdkaacEncoder_encode(encoder, pcm, [](void *ptr, ttLibC_Aac2 *aac) {
      int *counter = reinterpret_cast<int *>(ptr);
      *counter += 1;
      return true;
    }, &counter);
  }
  ttLibC_PcmS16_close(&pcm);
  ttLibC_FdkaacEncoder_close(&encoder);
  ttLibC_BeepGenerator_close(&generator);
  EXPECT_GT(counter, 2);
});

FDKAAC(FlvOutputTest, [this](){
  auto generator = ttLibC_BeepGenerator_make(PcmS16Type_littleEndian, 440, 44100, 1);
  auto encoder = ttLibC_FdkaacEncoder_make("AOT_AAC_LC", 44100, 1, 96000);
  ttLibC_PcmS16 *pcm = nullptr;
  typedef struct {
    ttLibC_FlvWriter *writer;
    FILE *fp;
    int counter;
  } holder_t;
  holder_t holder;
  holder.writer = ttLibC_FlvWriter_make(frameType_unknown, frameType_aac2);
  holder.fp = fopen("fdkaac_rec.flv", "wb");
  holder.counter = 0;
  if(holder.fp) {
    for(int i = 0;i < 5;++ i) {
      pcm = ttLibC_BeepGenerator_makeBeepBySampleNum(generator, pcm, 1000);
      ttLibC_FdkaacEncoder_encode(encoder, pcm, [](void *ptr, ttLibC_Aac2 *aac) {
        holder_t *holder = reinterpret_cast<holder_t *>(ptr);
        aac->inherit_super.inherit_super.id = 8;
        return ttLibC_FlvWriter_write(holder->writer, (ttLibC_Frame *)aac, [](void *ptr, void *data, size_t data_size){
          holder_t *holder = reinterpret_cast<holder_t *>(ptr);
          holder->counter ++;
          fwrite(data, 1, data_size, holder->fp);
          return true;
        }, ptr);
      }, &holder);
    }
    fclose(holder.fp);
  }
  ttLibC_PcmS16_close(&pcm);
  ttLibC_FlvWriter_close(&holder.writer);
  ttLibC_FdkaacEncoder_close(&encoder);
  ttLibC_BeepGenerator_close(&generator);
  EXPECT_GT(holder.counter, 2);
});

#undef Fdkaac
