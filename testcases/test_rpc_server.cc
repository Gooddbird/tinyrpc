#include <google/protobuf/service.h>
#include "../src/net/tcp/tcp_server.h"
#include "../src/net/net_address.h"
#include "../src/net/tinypb/tinypb_rpc_dispatcher.h"
#include "../src/comm/log.h"
#include "tinypb.pb.h"


class QueryServiceImpl : public QueryService {
 public:
  QueryServiceImpl() {}
  ~QueryServiceImpl() {}

  void query_name(google::protobuf::RpcController* controller,
                       const ::queryNameReq* request,
                       ::queryNameRes* response,
                       ::google::protobuf::Closure* done) {
    
    DebugLog << "========================";
    DebugLog << "this is query_name func";
    response->set_ret_code(0);
    response->set_res_info("OK");
    response->set_req_no(request->req_no());
    response->set_id(request->id());
    response->set_name("ikerli");
    
    DebugLog << "========================";
    done->Run();
  }
  void query_age(google::protobuf::RpcController* controller,
                       const ::queryAgeReq* request,
                       ::queryAgeRes* response,
                       ::google::protobuf::Closure* done) {

    DebugLog << "========================";
    DebugLog << "this is query_age func";
    response->set_ret_code(0);
    response->set_res_info("OK");
    response->set_req_no(request->req_no());
    response->set_id(request->id());
    response->set_age(20);
    DebugLog << "========================";
    done->Run();
  }

};

int main(int argc, char* argv[]) {

  tinyrpc::IPAddress::ptr addr = std::make_shared<tinyrpc::IPAddress>("127.0.0.1", 39999);
  
  tinyrpc::TcpServer server(addr, 1);
  tinyrpc::TinyPbRpcDispacther* dispatcher = server.getDispatcher();
  QueryService* service = new QueryServiceImpl();
  
  DebugLog << "================";
  dispatcher->registerService(service);

  server.start();
  return 0;
}
