#ifndef TINYRPC_NET_TINYPB_TINYPB_DATA_H
#define TINYRPC_NET_TINYPB_TINYPB_DATA_H

#include <stdint.h>
#include <vector>
#include <string>
#include "../abstract_data.h"

namespace tinyrpc {

struct TinyPbStruct : public AbstractData {

  TinyPbStruct() {};
  ~TinyPbStruct() {};
  
  // char* start;
  int32_t pk_len {-1};
  int32_t service_name_len {-1};
  std::string service_full_name;
  std::vector<char> pb_data {1};
  int32_t check_num {-1};
  // char end;

  bool parse_succ {false};

  CodeCType type() const {
    return CodeCType::CODEC_TINYPB;
  }
};

}

#endif
