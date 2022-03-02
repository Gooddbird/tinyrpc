#ifndef TINYRPC_NET_HTTP_HTTP_CODEC_H
#define TINYRPC_NET_HTTP_HTTP_CODEC_H

#include "../codec.h"

namespace tinyrpc {

class HttpCodeC : public CodeC {
 public:
  HttpCodeC();

  ~HttpCodeC();

  void encode(TcpBuffer& buf);
  
  void decode(TcpBuffer& buf);

  CodeCType type() const;

 private:

};

} 


#endif
