#ifndef TINYRPC_NET_TINYPB_TINYPB_CODEC_H
#define TINYRPC_NET_TINYPB_TINYPB_CODEC_H

#include <stdint.h>
#include "tinyrpc/net/abstract_codec.h"
#include "tinyrpc/net/abstract_data.h"
#include "tinyrpc/net/tinypb/tinypb_data.h"

namespace tinyrpc {


class TinyPbCodeC: public AbstractCodeC {
 public:
  // typedef std::shared_ptr<TinyPbCodeC> ptr;

  TinyPbCodeC();

  ~TinyPbCodeC ();

  // overwrite
  void encode(TcpBuffer* buf, AbstractData* data);
  
  // overwrite
  void decode(TcpBuffer* buf, AbstractData* data);

  // overwrite
  virtual ProtocalType getProtocalType();

  const char* encodePbData(TinyPbStruct* data, int& len);


};

} 


#endif
