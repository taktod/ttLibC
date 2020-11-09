/**
 * 
 */
#ifndef TTLIBCPP_CONTAINER_MKV_HPP_
#define TTLIBCPP_CONTAINER_MKV_HPP_

#include <ttLibC/container/mkv.h>

#include <functional>
#include <vector>

namespace ttLibCpp {

class TT_ATTRIBUTE_API MkvWriter {
public:
  static MkvWriter *create(std::vector<ttLibC_Frame_Type> types);
  static MkvWriter *create(ttLibC_Frame_Type videoType, ttLibC_Frame_Type audioType);
  MkvWriter();
  virtual ~MkvWriter();
  virtual bool write(ttLibC_Frame *frame, std::function<bool(void *data, size_t data_size)>) = 0;

  uint64_t pts;
  uint32_t unitDuration;
  uint32_t mode;
protected:
  uint32_t timebase;
};

}

#endif
