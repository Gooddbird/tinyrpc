#ifndef TINYRPC_NET_TINYPB_TINYPB_CODEC_H
#define TINYRPC_NET_TINYPB_TINYPB_CODEC_H

#include <stdint.h>
#include "../abstract_codec.h"
#include "../abstract_data.h"
#include "tinypb_data.h"

namespace tinyrpc {


class TinyPbCodeC: public AbstractCodeC {
 public:
  typedef std::shared_ptr<TinyPbCodeC> ptr;

  TinyPbCodeC();

  ~TinyPbCodeC ();

  // overwrite
  void encode(TcpBuffer* buf, AbstractData* data);
  
  // overwrite
  void decode(TcpBuffer* buf, AbstractData* data);

  const char* encodePbData(TinyPbStruct* data, int& len);

  CodeCType type() const;

};

} 


#endif
