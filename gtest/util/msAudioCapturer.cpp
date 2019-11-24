#include "../util.hpp"
#include <ttLibC/util/msAudioCapturerUtil.h>
#include <ttLibC/util/msGlobalUtil.h>
#include <ttLibC/encoder/msAacEncoder.h>
#include <ttLibC/container/flv.h>
#include <ttLibC/resampler/audioResampler.h>

using namespace std;

#ifdef __ENABLE_WIN32__
#define MSAUDIOCAPTURER(A, B) TEST_F(UtilTest, MsAudioCapturer##A){B();}
#else
#define MSAUDIOCAPTURER(A, B) TEST_F(UtilTest, DISABLED_MsAudioCapturer##A){}
#endif

MSAUDIOCAPTURER(Listup, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  int counter = 0;
  bool result = ttLibC_MsAudioCapturer_getDeviceNames([](void *ptr, const wchar_t *name){
    char buffer[256];
    ttLibC_MsGlobal_unicodeToSjis(name, buffer, 256);
    cout << buffer << endl;
    int *counter = reinterpret_cast<int *>(ptr);
    *counter += 1;
    return true;
  }, &counter);
  ASSERT_GT(counter, 0);
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSAUDIOCAPTURER(Capture, [this]() {
  // try to capture audio actually.
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  wstring name(L"");
  bool result = ttLibC_MsAudioCapturer_getDeviceNames([](void* ptr, const wchar_t* name) {
    wstring* str = reinterpret_cast<wstring*>(ptr);
    str->append(name);
    return false;
    }, &name);
  int counter = 0;
  auto capturer = ttLibC_MsAudioCapturer_make(name.c_str(), 48000, 2, [](void *ptr, ttLibC_Audio* audio) {
    int *counter = reinterpret_cast<int *>(ptr);
    *counter += 1;
    return true;
  }, &counter);
  if(capturer != nullptr) {
    ttLibC_MsGlobal_sleep(500);
    ttLibC_MsAudioCapturer_close(&capturer);
  }
  ASSERT_GT(counter, 0);
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSAUDIOCAPTURER(FlvOutputTest, [this](){
  // try to capture audio actually.
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  typedef struct {
    ttLibC_FlvWriter *writer;
    ttLibC_PcmS16 *pcm;
    FILE *fp;
    ttLibC_MsAacEncoder *encoder;
    int counter;
  } holder_t;
  int sample_rate = 48000;
  int channel_num = 2;
  holder_t holder;
  holder.writer = ttLibC_FlvWriter_make(frameType_unknown, frameType_aac);
  holder.counter = 0;
  holder.pcm = nullptr;
  holder.fp = fopen("msAudioCapturer_rec.flv", "wb");
  if(holder.fp) {
    holder.encoder = ttLibC_MsAacEncoder_make(sample_rate, channel_num, 96000);
    wstring name(L"");
    bool result = ttLibC_MsAudioCapturer_getDeviceNames([](void* ptr, const wchar_t* name) {
      wstring* str = reinterpret_cast<wstring*>(ptr);
      str->append(name);
      return false;
    }, &name);
    auto capturer = ttLibC_MsAudioCapturer_make(name.c_str(), sample_rate, channel_num, [](void *ptr, ttLibC_Audio* audio) {
      // ... this is pcmF32...
      holder_t *holder = reinterpret_cast<holder_t *>(ptr);
      auto p = ttLibC_AudioResampler_makePcmS16FromPcmF32(holder->pcm, PcmS16Type_littleEndian, (ttLibC_PcmF32 *)audio);
      if(p != nullptr) {
        holder->pcm = p;
      }
      return ttLibC_MsAacEncoder_encode(holder->encoder, holder->pcm, [](void* ptr, ttLibC_Aac* aac) {
        holder_t* holder = reinterpret_cast<holder_t *>(ptr);
        aac->inherit_super.inherit_super.id = 0x08;
        return ttLibC_FlvWriter_write(holder->writer, (ttLibC_Frame*)aac, [](void* ptr, void* data, size_t data_size) {
          holder_t* holder = reinterpret_cast<holder_t*>(ptr);
          fwrite(data, 1, data_size, holder->fp);
          holder->counter++;
          return true;
        }, ptr);
      }, ptr);
    }, &holder);
    if(capturer != nullptr) {
      ttLibC_MsGlobal_sleep(5000);
      ttLibC_MsAudioCapturer_close(&capturer);
    }
    fclose(holder.fp);
    ttLibC_MsAacEncoder_close(&holder.encoder);
    ttLibC_PcmS16_close(&holder.pcm);
  }
  ttLibC_FlvWriter_close(&holder.writer);
  ASSERT_GT(holder.counter, 0);
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});
