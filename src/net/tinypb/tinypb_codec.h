#ifndef TINYRPC_NET_TINYPB_TINYPB_CODEC_H
#define TINYRPC_NET_TINYPB_TINYPB_CODEC_H

#include <stdint.h>
#include "../abstract_codec.h"
#include "../abstract_data.h"

namespace tinyrpc {

class TinyPbCodeC: public AbstractCodeC {
 public:
  typedef std::shared_ptr<TinyPbCodeC> ptr;

  TinyPbCodeC();

  ~TinyPbCodeC ();

  void encode(TcpBuffer::ptr buf, AbstractData* data);
  
  void decode(TcpBuffer::ptr buf, AbstractData* data);

  void encodePb(TinyPbStruct* data);

  CodeCType type() const;

 private:

};

} 


#endif
