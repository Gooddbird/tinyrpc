#include <string.h>
#include <vector>
#include <algorithm>
#include "tinypb_codec.h"
#include "../byte.h"
#include "../../log/log.h"


namespace tinyrpc {

const char pb_start = 0x02;
const char pb_end = 0x03;



TinyPbCodeC::TinyPbCodeC() {

}

TinyPbCodeC::~TinyPbCodeC() {

}

void TinyPbCodeC::encode(TcpBuffer& buf) {
  DebugLog << "test encode";
  buf.readAble();
}

void TinyPbCodeC::decode(TcpBuffer& buf) {
  DebugLog << "test decode start";
  std::vector<char> tmp = buf.getBufferVector();
  int total_size = tmp.size();
  int start_index = -1;
  int end_index = -1;
  int32_t pk_len= -1; 
  int32_t service_name_len= -1; 

  bool parse_flag = false;
  
  for (int i = 0; i < total_size; ++i) {
    // first find start
    if (tmp[i] == pb_start) {
      if (i + 1 < total_size) {
        pk_len = getInt32FromNetByte(&tmp[i+1]);
        int j = i + pk_len - 1;

        if (j >= total_size) {
          DebugLog << "recv package not complete, or pk_start find error, continue next parse";
          continue;
        }
        if (tmp[j] == pb_end) {
          start_index = i;
          end_index = j;
          parse_flag = true;
          break;
        }
        
      }
      
    }
  }

  if (!parse_flag) {
    DebugLog << "parse error, return";
    return;
  }



  DebugLog << "test decode end";
}

CodeCType TinyPbCodeC::type() const {
  return CODEC_HTTP;
}

}
