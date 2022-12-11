#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <random>
#include "tinyrpc/comm/log.h"
#include "tinyrpc/comm/config.h"
#include "tinyrpc/comm/msg_req.h"


namespace tinyrpc {

extern tinyrpc::Config::ptr gRpcConfig;

static thread_local std::string t_msg_req_nu;
static thread_local std::string t_max_msg_req_nu;
// static thread_local int t_msg_req_len = 20;

static int g_random_fd = -1;

std::string MsgReqUtil::genMsgNumber() {

  int t_msg_req_len = 20;
  if (gRpcConfig) {
    t_msg_req_len = gRpcConfig->m_msg_req_len;
  }

  if (t_msg_req_nu.empty() || t_msg_req_nu == t_max_msg_req_nu) {
    if (g_random_fd == -1) {
      g_random_fd = open("/dev/urandom", O_RDONLY);
    } 
    std::string res(t_msg_req_len, 0);
    if ((read(g_random_fd, &res[0], t_msg_req_len)) != t_msg_req_len) {
      ErrorLog << "read /dev/urandom data less " << t_msg_req_len << " bytes";
      return "";
    }
    t_max_msg_req_nu = "";
    for (int i = 0; i < t_msg_req_len; ++i) {
      uint8_t x = ((uint8_t)(res[i])) % 10;
      res[i] = x + '0';
      t_max_msg_req_nu += "9";
    }
    t_msg_req_nu = res;

  } else {
    int i = t_msg_req_nu.length() - 1; 
    while(t_msg_req_nu[i] == '9' && i >= 0) {
      i--;
    }
    if (i >= 0) {
      t_msg_req_nu[i] += 1;
      for (size_t j = i + 1; j < t_msg_req_nu.length(); ++j) {
        t_msg_req_nu[j] = '0';
      }
    }

  }    
  // DebugLog << "get msg_req_nu is " << t_msg_req_nu;
  return t_msg_req_nu;
}

}