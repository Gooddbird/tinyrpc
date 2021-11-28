#ifndef TINYRPC_COMM_LOG_H
#define TINYRPC_COMM_LOG_H

#include <sstream>
#include <sstream>
#include <time.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <unistd.h>


#define LOG tinyrpc::log()

namespace tinyrpc {


static thread_local int g_thread_id = 0;


pid_t gettid() {
  return syscall(SYS_gettid);
}


std::stringstream log() {
  if (g_thread_id == 0) {
    g_thread_id = gettid(); 
  }

  std::stringstream ss;
  ss << "[" << time(NULL) << "]  [" << getpid() << "]  [" << g_thread_id << "]  [" << __FILE__ << "]  [" << __LINE__ << "]  ";
  return ss;
}

}

#endif
