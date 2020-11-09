/**
 * 
 */
#ifndef TTLIBCPP_UTIL_DYNAMICBUFFERUTIL_HPP_
#define TTLIBCPP_UTIL_DYNAMICBUFFERUTIL_HPP_

#include <ttLibC/util/dynamicBufferUtil.h>

namespace ttLibCpp {

class TT_ATTRIBUTE_API DynamicBuffer {
public:
  static DynamicBuffer *create();
  DynamicBuffer();
  virtual ~DynamicBuffer();

  virtual bool append(uint8_t *data, size_t dataSize) = 0;
  virtual bool markAsRead(size_t readSize) = 0;
  virtual uint8_t *refData() = 0;
  virtual size_t refSize() = 0;
  virtual bool reset() = 0;
  virtual bool clear() = 0;
  virtual bool empty() = 0;
  virtual bool alloc(size_t size) = 0;
  virtual bool write(size_t writePos, uint8_t *data, size_t dataSize) = 0;

  size_t targetSize;
};

}

#endif
