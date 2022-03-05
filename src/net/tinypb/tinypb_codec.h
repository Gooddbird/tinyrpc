#ifndef TINYRPC_NET_TINYPB_TINYPB_CODEC_H
#define TINYRPC_NET_TINYPB_TINYPB_CODEC_H

#include <stdint.h>
#include "../codec.h"
#include "../abstract_data.h"

namespace tinyrpc {

class TinyPbCodeC: public CodeC {
 public:
  typedef std::shared_ptr<TinyPbCodeC> ptr;

  TinyPbCodeC();

  ~TinyPbCodeC ();

  void encode(TcpBuffer::ptr buf, AbstractData* data);
  
  void decode(TcpBuffer::ptr buf, AbstractData* data);

  CodeCType type() const;

 private:

};

} 


#endif
