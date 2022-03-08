#ifndef TINYRPC_NET_HTTP_HTTP_CODEC_H
#define TINYRPC_NET_HTTP_HTTP_CODEC_H

#include "../abstract_data.h"
#include "../abstract_codec.h"

namespace tinyrpc {

class HttpCodeC : public AbstractCodeC {
 public:
  HttpCodeC();

  ~HttpCodeC();

  void encode(TcpBuffer* buf, AbstractData* data);
  
  void decode(TcpBuffer* buf, AbstractData* data);

  CodeCType type() const;

 private:

};

} 


#endif
