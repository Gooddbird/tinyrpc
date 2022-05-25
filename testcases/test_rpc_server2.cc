#include <google/protobuf/service.h>
#include <iostream>
#include "tcp_server.h"
#include "net_address.h"
#include "tinypb_rpc_dispatcher.h"
#include "tinypb_rpc_channel.h"
#include "tinypb_rpc_closure.h"
#include "tinypb_rpc_controller.h"
#include "log.h"
#include "coroutine_pool.h"
#include "tinypb.pb.h"
#include "tinyrpc/comm/start.h"

int n = 10;

void fun() {
  tinyrpc::IPAddress::ptr peer_addr = std::make_shared<tinyrpc::IPAddress>("127.0.0.1", 39999);
  tinyrpc::TinyPbRpcChannel channel(peer_addr);
  DebugLog << "input an integer to set count that send tinypb data";
  while (n--) {

    DebugLog << "==========================test no:" << n;
    queryNameReq req_name;
    req_name.set_req_no(20220315);
    req_name.set_id(1100110001);
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
    rpc_controller.SetTimeout(5000);
    stub.query_name(&rpc_controller, &req_name, &res_name, &cb);

    if (rpc_controller.ErrorCode() != 0) {
      ErrorLog << "call rpc method query_name failed, errcode=" << rpc_controller.ErrorCode() << ",error=" << rpc_controller.ErrorText();
    }
    if (res_name.ret_code() != 0) {
      ErrorLog << "query name error, errcode=" << res_name.ret_code() << ", res_info=" << res_name.res_info(); 
    } else {
      DebugLog << "get res_name.age = " << res_name.name();
    }

  }

  DebugLog << "================";

}


int main(int argc, char* argv[]) {

  if (argc != 2) {
    printf("Start TinyRPC server error, input argc is not 2!");
    printf("Start TinyRPC server like this: \n");
    printf("./server a.xml\n");
    return 0;
  }

  tinyrpc::InitConfig(argv[1]);

  tinyrpc::Coroutine::ptr cor = tinyrpc::GetCoroutinePool()->getCoroutineInstanse();
  cor->setCallBack(&fun);

  tinyrpc::GetServer()->addCoroutine(cor);

  tinyrpc::StartRpcServer();

  return 0;
}
