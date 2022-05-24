#include <google/protobuf/service.h>
#include "tcp_server.h"
#include "net_address.h"
#include "tinypb_rpc_dispatcher.h"
#include "log.h"
#include "tinyrpc/comm/start.h"
#include "tinypb.pb.h"
#include "coroutine_hook.h"
#include "tinyrpc/comm/start.h"
#include "tinyrpc/comm/mysql_instase.h"
#include "tinyrpc/comm/start.h"
#include <mysql/mysql.h>
#include <sstream>

int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("Start TinyRPC server error, input argc is not 2!");
    printf("Start TinyRPC server like this: \n");
    printf("./server a.xml\n");
    return 0;
  }

  tinyrpc::InitConfig(argv[1]);

  tinyrpc::StartRpcServer();
  
  return 0;
}
