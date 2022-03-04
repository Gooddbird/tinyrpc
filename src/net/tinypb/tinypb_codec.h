#ifndef TINYRPC_NET_TINYPB_TINYPB_CODEC_H
#define TINYRPC_NET_TINYPB_TINYPB_CODEC_H

#include <stdint.h>
#include "../codec.h"

namespace tinyrpc {

struct TinyPbStruct {
  // char* start;
  int32_t pk_len;
  int32_t service_name_len;
  std::string service_name;
  std::vector<char> pb_data {1};
  int32_t check_num;
  // char end;
};


class TinyPbCodeC: public CodeC {
 public:
  typedef std::shared_ptr<TinyPbCodeC> ptr;

  TinyPbCodeC();

  ~TinyPbCodeC ();

  void encode(TcpBuffer::ptr buf);
  
  void decode(TcpBuffer::ptr buf);

  CodeCType type() const;

 private:

};

} 


#endif
