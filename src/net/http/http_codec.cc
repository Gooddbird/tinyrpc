#include "http_codec.h"
#include "../../log/log.h"
#include "../abstract_data.h"

namespace tinyrpc {

HttpCodeC::HttpCodeC() {

}

HttpCodeC::~HttpCodeC() {

}

void HttpCodeC::encode(TcpBuffer::ptr buf, AbstractData* data) {
  DebugLog << "test encode";
  buf->readAble();
}

void HttpCodeC::decode(TcpBuffer::ptr buf, AbstractData* data) {

  DebugLog << "test decode start";
  buf->readAble();


  DebugLog << "test decode end";
}

CodeCType HttpCodeC::type() const {
  return CODEC_HTTP;
}

}
