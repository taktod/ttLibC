#include "../writer.hpp"

#include <ttLibC/container/mkv.h>
#include <ttLibCpp/container/mkv.hpp>

#define MKV(A, B) TEST_F(WriterTest, Mkv##A){B();}

MKV(H264AacTest, [this](){
  typedef struct {
    ttLibC_MkvReader *reader;
    ttLibC_MkvWriter *writer;
    FILE *fp;
    FILE *fp_out;
    uint32_t count;
  } DataHolder_t;

	char file[256];
	sprintf(file, "%s/tools/data/source/test.h264.aac.mkv", getenv("HOME"));
  ttLibC_Frame_Type types[2];
  types[0] = frameType_h264;
  types[1] = frameType_aac2;
  DataHolder_t holder = (DataHolder_t{
    ttLibC_MkvReader_make(),
    ttLibC_MkvWriter_make(types, 2),
    fopen(file, "rb"),
    fopen("output.h264.aac.mkv", "wb"),
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
          return ttLibC_MkvWriter_write(holder->writer, frame, [](void *ptr, void *data, size_t data_size) {
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
  ttLibC_MkvReader_close(&holder.reader);
  ttLibC_MkvWriter_close(&holder.writer);
});

using namespace ttLibCpp;

MKV(CppH265Mp3Test, [this](){
  typedef struct {
    ttLibC_MkvReader *reader;
    MkvWriter *writer;
    FILE *fp;
    FILE *fp_out;
    uint32_t count;
  } DataHolder_t;

  char file[256];
  sprintf(file, "%s/tools/data/source/test.h265.mp3.mkv", getenv("HOME"));
  DataHolder_t holder = (DataHolder_t{
    ttLibC_MkvReader_make(),
    MkvWriter::create({frameType_h265, frameType_mp3}),
    fopen(file, "rb"),
    fopen("output.h265.mp3.mkv", "wb"),
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
          return holder->writer->write(frame, [&](void *data, size_t data_size) {
            fwrite(data, 1, data_size, holder->fp_out);
            return true;
          });
        }, ptr);
      }, &holder)) {
        break;
      }
    } while(!feof(holder.fp));
    fclose(holder.fp);
  }
  fclose(holder.fp_out);
  ttLibC_MkvReader_close(&holder.reader);
  if(holder.writer) {
    delete holder.writer;
  }
});

MKV(CppH264AacTest, [this](){
  typedef struct {
    ttLibC_MkvReader *reader;
    MkvWriter *writer;
    FILE *fp;
    FILE *fp_out;
    uint32_t count;
  } DataHolder_t;

  char file[256];
  sprintf(file, "%s/tools/data/source/test.h264.aac.mkv", getenv("HOME"));
  DataHolder_t holder = (DataHolder_t{
    ttLibC_MkvReader_make(),
    MkvWriter::create(frameType_h264, frameType_aac2),
    fopen(file, "rb"),
    fopen("output.h264.aac.mkv", "wb"),
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
          return holder->writer->write(frame, [&](void *data, size_t data_size) {
            fwrite(data, 1, data_size, holder->fp_out);
            return true;
          });
        }, ptr);
      }, &holder)) {
        break;
      }
    } while(!feof(holder.fp));
    fclose(holder.fp);
  }
  fclose(holder.fp_out);
  ttLibC_MkvReader_close(&holder.reader);
  if(holder.writer) {
    delete holder.writer;
  }
});

#undef MKV
