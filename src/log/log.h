#ifndef TINYRPC_LOG_LOG_H
#define TINYRPC_LOG_LOG_H

#include <sstream>
#include <sstream>
#include <time.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <unistd.h>

namespace tinyrpc {

#define LOG log()

std::stringstream& log() {
  std::stringstream ss;
  ss << "[" << time(NULL) << "]  [" << getpid() << "]  [" << syscall(SYS_gettid) << "]  [" << __FILE__ << "]  [" << __LINE__ << "]  ";
  return ss;
}

}

#endif
