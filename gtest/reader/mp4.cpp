#include "../reader.hpp"

#include <ttLibC/container/mp4.h>

#define MP4(A, B) TEST_F(ReaderTest, Mp4##A){B();}

MP4(H264AacTest, [this]() {
  typedef struct {
    ttLibC_Mp4Reader *reader;
    FILE *fp;
    uint32_t h264;
    uint32_t aac;
  } DataHolder_t;

  char file[256];
	sprintf(file, "%s/tools/data/source/test.h264.aac.mp4", getenv("HOME"));
  DataHolder_t holder = (DataHolder_t{
    ttLibC_Mp4Reader_make(),
    fopen(file, "rb"),
    0,
    0});
  if(holder.fp) {
    do {
      uint8_t buffer[65536];
      if(!holder.fp) {
        break;
      }
      size_t read_size = fread(buffer, 1, 65536, holder.fp);
      if(!ttLibC_Mp4Reader_read(holder.reader, buffer, read_size, [](void *ptr, ttLibC_Mp4 *mp4){
        return ttLibC_Mp4_getFrame(mp4, [](void *ptr, ttLibC_Frame *frame){
          DataHolder_t *holder = (DataHolder_t *)ptr;
          switch(frame->type) {
          case frameType_aac:
//            holder->aac ++;
            break;
          case frameType_aac2:
            holder->aac ++;
            break;
          case frameType_h264:
            holder->h264 ++;
            break;
          default:
            break;
          }
          return true;
        }, ptr);
      }, &holder)) {
        break;
      }
    } while(!feof(holder.fp));
    fclose(holder.fp);
  }
  ttLibC_Mp4Reader_close(&holder.reader);
  ASSERT_GT(holder.h264, 0);
  ASSERT_GT(holder.aac, 0);
});

#undef MP4