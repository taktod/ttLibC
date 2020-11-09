/**
 * 
 */
#ifndef TTLIBCPP_UTIL_BYTEUTIL_HPP_
#define TTLIBCPP_UTIL_BYTEUTIL_HPP_

#include <ttLibC/util/byteUtil.h>
#include <string>

namespace ttLibCpp {

class TT_ATTRIBUTE_API ByteConnector {
public:
  static ByteConnector *create(void *data, size_t dataSize, ttLibC_ByteUtil_Type type);
  ByteConnector();
  virtual ~ByteConnector();
  virtual ByteConnector *bit(uint32_t value, uint32_t bitNum) = 0;
  virtual ByteConnector *ebml(uint32_t value, uint32_t isTag) = 0;
  virtual ByteConnector *string(std::string str) = 0;

  size_t writeSize;
};

}

#endif
