#include "http_codec.h"
#include "tinyrpc/comm/log.h"
#include "tinyrpc/net/abstract_data.h"
#include "tinyrpc/net/abstract_codec.h"

namespace tinyrpc {

HttpCodeC::HttpCodeC() {

}

HttpCodeC::~HttpCodeC() {

}

void HttpCodeC::encode(TcpBuffer* buf, AbstractData* data) {
  DebugLog << "test encode";
  buf->readAble();
}

void HttpCodeC::decode(TcpBuffer* buf, AbstractData* data) {

  DebugLog << "test decode start";
  buf->readAble();


  DebugLog << "test decode end";
}

}
