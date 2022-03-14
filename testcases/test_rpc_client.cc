#include <google/protobuf/service.h>
#include "../src/net/tcp/tcp_server.h"
#include "../src/net/net_address.h"
#include "../src/net/tinypb/tinypb_rpc_dispatcher.h"
#include "../src/net/tinypb/tinypb_rpc_channel.h"
#include "../src/net/tinypb/tinypb_rpc_closure.h"
#include "../src/net/tinypb/tinypb_rpc_controller.h"
#include "../src/log/log.h"
#include "tinypb.pb.h"


// class QueryServiceImpl : public QueryService {
//  public:
//   QueryServiceImpl() {}
//   ~QueryServiceImpl() {}

//   void query_name(google::protobuf::RpcController* controller,
//                        const ::QueryReq* request,
//                        ::QueryNameRes* response,
//                        ::google::protobuf::Closure* done) {
    
//     DebugLog << "========================";
//     DebugLog << "this is query_name func";
//     response->set_name("ikerli");
//     DebugLog << "========================";
//     done->Run();
//   }
//   void query_age(google::protobuf::RpcController* controller,
//                        const ::QueryReq* request,
//                        ::QueryAgeRes* response,
//                        ::google::protobuf::Closure* done) {

//     DebugLog << "========================";
//     DebugLog << "this is query_age func";
//     response->set_age(20);
//     DebugLog << "========================";
//   }

// };

int main(int argc, char* argv[]) {

  tinyrpc::IPAddress::ptr addr = std::make_shared<tinyrpc::IPAddress>("127.0.0.1", 39999);
  
  tinyrpc::TinyPbRpcChannel channel(addr);
  tinyrpc::TinyPbRpcController rpc_controller;
  DebugLog << "input an integer to set count that send tinypb data";
  int n;
  std::cin >> n;

  while (n--) {

  QueryReq req;
  QueryAgeRes res_age;
  QueryNameRes res_name;

  tinyrpc::TinyPbRpcClosure cb([]() {
    DebugLog << "==========================";
    DebugLog << "succ call rpc";
    DebugLog << "==========================";
  });

  QueryService_Stub stub(&channel);
  req.set_id(20200312);
  stub.query_name(&rpc_controller, &req, &res_name, &cb);
  stub.query_name(&rpc_controller, &req, &res_name, &cb);
  stub.query_age(&rpc_controller, &req, &res_age, &cb);

  }

  DebugLog << "================";
  return 0;
}
