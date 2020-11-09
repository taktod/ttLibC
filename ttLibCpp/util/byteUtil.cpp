#include "byteUtil.hpp"
#include <ttLibC/util/byteUtil.h>

namespace ttLibCpp {

class ByteConnector_ : public ByteConnector {
public:
  ByteConnector_(void *data, size_t dataSize, ttLibC_ByteUtil_Type type) : connector(ttLibC_ByteConnector_make(data, dataSize, type)){
  }
  ~ByteConnector_() {
    ttLibC_ByteConnector_close(&connector);
  }
  ByteConnector *bit(uint32_t value, uint32_t bitNum) {
    bool result = ttLibC_ByteConnector_bit(connector, value, bitNum);
    writeSize = connector->write_size;
    return this;
  }
  ByteConnector *ebml(uint32_t value, uint32_t isTag) {
    bool result = ttLibC_ByteConnector_ebml2(connector, value, isTag);
    writeSize = connector->write_size;
    return this;
  }
  ByteConnector *string(std::string str) {
    bool result = ttLibC_ByteConnector_string(connector, str.c_str(), str.length());
    writeSize = connector->write_size;
    return this;
  }
  ttLibC_ByteConnector *connector;
};

ByteConnector::ByteConnector() : writeSize(0) {
}

ByteConnector::~ByteConnector() {
}

ByteConnector *ByteConnector::create(void *data, size_t dataSize, ttLibC_ByteUtil_Type type) {
  return new ByteConnector_(data, dataSize, type);
}

}
