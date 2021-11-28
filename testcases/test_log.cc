#include "../src/comm/log.h"
#include <unistd.h>

int main(int argc, char** argv) {
  
  DebugLog << "this is test log 1";
  sleep(2);
  InfoLog << "this is test log 2";
  WarnLog << "this is test log 3";
  ErrorLog << "this is test log 4";
  return 0;
}
