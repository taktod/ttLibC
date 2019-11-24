#include "../reader.hpp"

#include <ttLibC/container/mkv.h>

#define MKV(A, B) TEST_F(ReaderTest, Mkv##A){B();}

MKV(H264AacTest, [this]() {
  typedef struct {
    ttLibC_MkvReader *reader;
    FILE *fp;
    uint32_t h264;
    uint32_t aac;
  } DataHolder_t;

  char file[256];
	sprintf(file, "%s/tools/data/source/test.h264.aac.mkv", getenv("HOME"));
  DataHolder_t holder = (DataHolder_t{
    ttLibC_MkvReader_make(),
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
      if(!ttLibC_MkvReader_read(holder.reader, buffer, read_size, [](void *ptr, ttLibC_Mkv *mkv){
        return ttLibC_Mkv_getFrame(mkv, [](void *ptr, ttLibC_Frame *frame){
          DataHolder_t *holder = (DataHolder_t *)ptr;
          switch(frame->type) {
          case frameType_aac:
            holder->aac ++;
            break;
          case frameType_aac2:
//            puts("aac2 found");
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
  ttLibC_MkvReader_close(&holder.reader);
  ASSERT_GT(holder.h264, 0);
  ASSERT_GT(holder.aac, 0);
});

#undef MKV