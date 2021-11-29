#include "../src/log/log.h"
#include <unistd.h>


void* fun(void* arg) {

  DebugLog << "this is test log 1";

  return nullptr;
}

int main(int argc, char** argv) {
  
  DebugLog << "this is test log 1";
  sleep(2);
  InfoLog << "this is test log 2";
  WarnLog << "this is test log 3";
  ErrorLog << "this is test log 4";
  
  pthread_t tid;
  pthread_create(&tid, nullptr, fun, nullptr);
  pthread_join(tid, nullptr);



  return 0;
}
