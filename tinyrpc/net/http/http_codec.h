#ifndef TINYRPC_NET_HTTP_HTTP_CODEC_H
#define TINYRPC_NET_HTTP_HTTP_CODEC_H

#include "tinyrpc/net/abstract_data.h"
#include "tinyrpc/net/abstract_codec.h"
#include "tinyrpc/net/http/http_request.h"

namespace tinyrpc {


class HttpCodeC : public AbstractCodeC {
 public:
  HttpCodeC();

  ~HttpCodeC();

  void encode(TcpBuffer* buf, AbstractData* data);
  
  void decode(TcpBuffer* buf, AbstractData* data);

 private:
  bool parseHttpRequestLine(HttpRequest* requset, int i);
  bool parseHttpRequestHeader(HttpRequest* requset, int i);

 private:
  std::string m_strs;
};

} 


#endif
