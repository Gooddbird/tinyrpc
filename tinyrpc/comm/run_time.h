#ifndef TINYRPC_COMM_RUN_TIME_H
#define TINYRPC_COMM_RUN_TIME_H

#include <string>

namespace tinyrpc {

class RunTime {
 public:
  std::string m_msg_no;
  std::string m_interface_name;
};

}


#endif