#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <tinyxml/tinyxml.h>
#include "tinyrpc/comm/start.h"
#include "tinyrpc/comm/config.h"
#include "tinyrpc/comm/log.h"
#include "tinyrpc/net/ftp/ftp_server.h"
#include "tinyrpc/net/net_address.h"

extern tinyrpc::Logger::ptr gRpcLogger;

int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("Usage: test_ftp_server <config_file>\n");
    return 1;
  }

  tinyrpc::InitConfig(argv[1]);

  
  tinyrpc::StartRpcServer();
  return 0;
}
