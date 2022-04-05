#ifndef TINYRPC_LOG_LOG_BUFFER_H
#define TINYRPC_LOG_LOG_BUFFER_H

#include <queue>
#include <string>
#include <memory>

namespace tinyrpc {

class LogBuffer {
 public:
  typedef std::shared_ptr<LogBuffer> ptr;

  void push(const std::string& item);

 private:
  std::queue<std::string> m_buf;
};



}




#endif  // TINYRPC_LOG_LOG_BUFFER_H