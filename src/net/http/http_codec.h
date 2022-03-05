#ifndef TINYRPC_NET_HTTP_HTTP_CODEC_H
#define TINYRPC_NET_HTTP_HTTP_CODEC_H

#include "../codec.h"

namespace tinyrpc {

class HttpCodeC : public CodeC {
 public:
  HttpCodeC();

  ~HttpCodeC();

  void encode(TcpBuffer::ptr buf, AbstractData* data);
  
  void decode(TcpBuffer::ptr buf, AbstractData* data);

  CodeCType type() const;

 private:

};

} 


#endif
