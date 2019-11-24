#include "../writer.hpp"

#include <ttLibC/container/flv.h>

#define FLV(A, B) TEST_F(WriterTest, Flv##A){B();}

FLV(H264AacTest, [this](){
  typedef struct {
    ttLibC_FlvReader *reader;
    ttLibC_FlvWriter *writer;
    FILE *fp;
    FILE *fp_out;
    uint32_t count;
  } DataHolder_t;

	char file[256];
	sprintf(file, "%s/tools/data/source/test.h264.aac.flv", getenv("HOME"));
  DataHolder_t holder = (DataHolder_t{
    ttLibC_FlvReader_make(),
    ttLibC_FlvWriter_make(frameType_h264, frameType_aac2),
    fopen(file, "rb"),
    fopen("output.h264.aac.flv", "wb"),
    0});
  if(holder.fp) {
    do {
      uint8_t buffer[65536];
      if(!holder.fp) {
        break;
      }
      size_t read_size = fread(buffer, 1, 65536, holder.fp);
      if(!ttLibC_FlvReader_read(holder.reader, buffer, read_size, [](void *ptr, ttLibC_Flv *flv){
        return ttLibC_Flv_getFrame(flv, [](void *ptr, ttLibC_Frame *frame){
          DataHolder_t *holder = (DataHolder_t *)ptr;
          return ttLibC_FlvWriter_write(holder->writer, frame, [](void *ptr, void *data, size_t data_size) {
            DataHolder_t *holder = (DataHolder_t *)ptr;
            fwrite(data, 1, data_size, holder->fp_out);
            return true;
          }, ptr);
        }, ptr);
      }, &holder)) {
        break;
      }
    } while(!feof(holder.fp));
    fclose(holder.fp);
  }
  fclose(holder.fp_out);
  ttLibC_FlvReader_close(&holder.reader);
  ttLibC_FlvWriter_close(&holder.writer);
});

#undef FLV
