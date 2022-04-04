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

  /*
  **  min of package is: 1 + 4 + 4 + 4 + 4 + 4 + 4 + 1 = 26 bytes
  **
  */
  
  // char start;                      // indentify start of a TinyPb protocal data
  int32_t pk_len {0};                 // len of all package(include start char and end char)
  int32_t msg_req_len {0};            // len of msg_req
  std::string msg_req;                // msg_req, which identify a request 
  int32_t service_name_len {0};       // len of service full name
  std::string service_full_name;      // service full name, like QueryService.query_name
  int32_t err_code {0};               // err_code, 0 -- call rpc success, otherwise -- call rpc failed. it only be seted by RpcController
  int32_t err_info_len {0};           // len of err_info
  std::string err_info;               // err_info, empty -- call rpc success, otherwise -- call rpc failed, it will display details of reason why call rpc failed. it only be seted by RpcController
  std::string pb_data {"1"};          // business pb data
  int32_t check_num {-1};             // check_num of all package. to check legality of data
  // char end;                        // identify end of a TinyPb protocal data

  CodeCType type() const {
    return CodeCType::CODEC_TINYPB;
  }
};

}

#endif
