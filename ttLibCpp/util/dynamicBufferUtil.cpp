#include "dynamicBufferUtil.hpp"
#include <ttLibC/util/dynamicBufferUtil.h>

namespace ttLibCpp {

class DynamicBuffer_ : public DynamicBuffer {
public:
  DynamicBuffer_() : buffer(ttLibC_DynamicBuffer_make()) {
  }
  ~DynamicBuffer_() {
    ttLibC_DynamicBuffer_close(&buffer);
  }
  bool append(uint8_t *data, size_t dataSize) {
    bool result = ttLibC_DynamicBuffer_append(buffer, data, dataSize);
    targetSize = buffer->target_size;
    return result;
  }
  bool markAsRead(size_t readSize) {
    bool result = ttLibC_DynamicBuffer_markAsRead(buffer, readSize);
    targetSize = buffer->target_size;
    return result;
  }
  uint8_t *refData() {
    return ttLibC_DynamicBuffer_refData(buffer);
  }
  size_t refSize() {
    return ttLibC_DynamicBuffer_refSize(buffer);
  }
  bool reset() {
    bool result = ttLibC_DynamicBuffer_reset(buffer);
    targetSize = buffer->target_size;
    return result;
  }
  bool clear() {
    bool result = ttLibC_DynamicBuffer_clear(buffer);
    targetSize = buffer->target_size;
    return result;
  }
  bool empty() {
    bool result = ttLibC_DynamicBuffer_empty(buffer);
    targetSize = buffer->target_size;
    return result;
  }
  bool alloc(size_t size) {
    bool result = ttLibC_DynamicBuffer_alloc(buffer, size);
    targetSize = buffer->target_size;
    return result;
  }
  bool write(size_t writePos, uint8_t *data, size_t dataSize) {
    bool result = ttLibC_DynamicBuffer_write(buffer, writePos, data, dataSize);
    targetSize = buffer->target_size;
    return result;
  }
private:
  ttLibC_DynamicBuffer *buffer;
};

DynamicBuffer::DynamicBuffer() : targetSize(0) {
}

DynamicBuffer::~DynamicBuffer() {}

DynamicBuffer *DynamicBuffer::create() {
  return new DynamicBuffer_();
}

}

