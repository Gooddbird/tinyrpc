#include <iostream>
#include <google/protobuf/service.h>
#include "tinyrpc/net/tinypb/tinypb_rpc_channel.h"
#include "tinyrpc/net/tinypb/tinypb_rpc_async_channel.h"
#include "tinyrpc/net/tinypb/tinypb_rpc_controller.h"
#include "tinyrpc/net/tinypb/tinypb_rpc_closure.h"
#include "tinyrpc/net/net_address.h"
#include "test_tinypb_server.pb.h"

void test_client() {

  tinyrpc::IPAddress::ptr addr = std::make_shared<tinyrpc::IPAddress>("127.0.0.1", 39999);

  tinyrpc::TinyPbRpcChannel channel(addr);
  QueryService_Stub stub(&channel);

  tinyrpc::TinyPbRpcController rpc_controller;
  rpc_controller.SetTimeout(5000);

  queryAgeReq rpc_req;
  queryAgeRes rpc_res;

  std::cout << "Send to tinyrpc server " << addr->toString() << ", requeset body: " << rpc_req.ShortDebugString() << std::endl;
  stub.query_age(&rpc_controller, &rpc_req, &rpc_res, NULL);

  if (rpc_controller.ErrorCode() != 0) {
    std::cout << "Failed to call tinyrpc server, error code: " << rpc_controller.ErrorCode() << ", error info: " << rpc_controller.ErrorText() << std::endl; 
    return;
  }

  std::cout << "Success get response frrom tinyrpc server " << addr->toString() << ", response body: " << rpc_res.ShortDebugString() << std::endl;

} 

int main(int argc, char* argv[]) {

  test_client();

  return 0;
}
