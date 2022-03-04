#ifndef TINYRPC_NET_TINYPB_TINYPB_CODEC_H
#define TINYRPC_NET_TINYPB_TINYPB_CODEC_H

#include "../codec.h"

namespace tinyrpc {

struct TinyPbDefine {

};


class TinyPbCodeC: public CodeC {
 public:
  TinyPbCodeC();

  ~TinyPbCodeC ();

  void encode(TcpBuffer& buf);
  
  void decode(TcpBuffer& buf);

  CodeCType type() const;

 private:

};

} 


#endif
