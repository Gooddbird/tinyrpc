#include "http_codec.h"
#include "../../log/log.h"

namespace tinyrpc {

HttpCodeC::HttpCodeC() {

}

HttpCodeC::~HttpCodeC() {

}

void HttpCodeC::encode(TcpBuffer::ptr buf) {
  DebugLog << "test encode";
  buf->readAble();
}

void HttpCodeC::decode(TcpBuffer::ptr buf) {

  DebugLog << "test decode start";
  buf->readAble();


  DebugLog << "test decode end";
}

CodeCType HttpCodeC::type() const {
  return CODEC_HTTP;
}

}
