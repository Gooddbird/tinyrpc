#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <random>
#include "log.h"
#include "msg_req.h"

namespace tinyrpc {

static int g_random_fd = -1;

std::string genMsgNumber(const int len) {
  if (len == 0) {
    DebugLog << "genMsgNumber len == 0, return";
    return "";
  }
  if (g_random_fd == -1) {
    g_random_fd = open("/dev/urandom", O_RDONLY);
  } 
  std::string res(len, 0);
  if ((read(g_random_fd, &res[0], len)) != len) {
    ErrorLog << "read /dev/urandom data less " << len << "bytes";
    return "";
  }
  for (int i = 0; i < len; ++i) {
    uint8_t x = ((uint8_t)(res[i])) % 10;
    res[i] = x + '0';
  }
  return res;
}

}