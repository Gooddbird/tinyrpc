#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <random>
#include "log.h"
#include "msg_req.h"

namespace tinyrpc {

std::string genMsgNumber(const int len) {
  if (len == 0) {
    DebugLog << "genMsgNumber len == 0, return";
    return "";
  }
  std::random_device rd;
  int fd = open("/dev/urandom", O_RDONLY);
  std::string res(len, 0);
  if ((read(fd, &res[0], len)) != len) {
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