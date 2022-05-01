#include <google/protobuf/service.h>
#include <iostream>
#include "../src/net/tcp/tcp_server.h"
#include "../src/net/net_address.h"
#include "../src/net/tinypb/tinypb_rpc_dispatcher.h"
#include "../src/net/tinypb/tinypb_rpc_channel.h"
#include "../src/net/tinypb/tinypb_rpc_closure.h"
#include "../src/net/tinypb/tinypb_rpc_controller.h"
#include "../src/comm/log.h"
#include "../src/coroutine/coroutine_pool.h"
#include "tinypb.pb.h"


void fun() {
  tinyrpc::IPAddress::ptr peer_addr = std::make_shared<tinyrpc::IPAddress>("127.0.0.1", 39999);
  tinyrpc::TinyPbRpcChannel channel(peer_addr);
  DebugLog << "input an integer to set count that send tinypb data";
  int n;
  std::cin >> n;

  while (n--) {

    DebugLog << "==========================test no:" << n;
    queryNameReq req_name;
    req_name.set_req_no(20220315);
    req_name.set_id(1234);
    req_name.set_type(1);

    queryNameRes res_name;

    queryAgeReq req_age;
    req_age.set_req_no(00001111);
    req_age.set_id(6781);

    queryAgeRes res_age;

    tinyrpc::TinyPbRpcClosure cb([]() {
      DebugLog << "==========================";
      DebugLog << "succ call rpc";
      DebugLog << "==========================";
    });

    QueryService_Stub stub(&channel);
    tinyrpc::TinyPbRpcController rpc_controller;
    rpc_controller.SetTimeout(10000);
    stub.query_name(&rpc_controller, &req_name, &res_name, &cb);

    if (rpc_controller.ErrorCode() != 0) {
      ErrorLog << "call rpc method query_name failed, errcode=" << rpc_controller.ErrorCode() << ",error=" << rpc_controller.ErrorText();
    } else {
      DebugLog << "get res_name.age = " << res_name.name();
    }

  }

  DebugLog << "================";

}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cout << "use example:  ./out [port]" << std::endl;
    std::cout << "./out 30001" << std::endl;
    return 0;
  }

  int port = std::atoi(argv[1]);
  tinyrpc::IPAddress::ptr self_addr = std::make_shared<tinyrpc::IPAddress>("127.0.0.1", port);
  tinyrpc::TcpServer server(self_addr, 1);
  tinyrpc::Coroutine::ptr cor = tinyrpc::GetCoroutinePool()->getCoroutineInstanse();
  cor->setCallBack(&fun);
  server.addCoroutine(cor);

  server.start();

  return 0;
}
