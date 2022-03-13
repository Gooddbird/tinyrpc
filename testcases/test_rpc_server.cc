#include <google/protobuf/service.h>
#include "../src/net/tcp/tcp_server.h"
#include "../src/net/net_address.h"
#include "../src/net/tinypb/tinypb_rpc_dispatcher.h"
#include "../src/log/log.h"
#include "tinypb.pb.h"


class QueryServiceImpl : public QueryService {
 public:
  QueryServiceImpl() {}
  ~QueryServiceImpl() {}

  void query_name(google::protobuf::RpcController* controller,
                       const ::QueryReq* request,
                       ::QueryNameRes* response,
                       ::google::protobuf::Closure* done) {
    
    DebugLog << "========================";
    DebugLog << "this is query_name func";
    response->set_name("ikerli");
    DebugLog << "========================";
    done->Run();
  }
  void query_age(google::protobuf::RpcController* controller,
                       const ::QueryReq* request,
                       ::QueryAgeRes* response,
                       ::google::protobuf::Closure* done) {

    DebugLog << "========================";
    DebugLog << "this is query_age func";
    response->set_age(20);
    DebugLog << "========================";
    done->Run();
  }

};

int main(int argc, char* argv[]) {

  tinyrpc::IPAddress::ptr addr = std::make_shared<tinyrpc::IPAddress>("127.0.0.1", 39999);
  
  tinyrpc::TcpServer server(addr);
  tinyrpc::TinyPbRpcDispacther* dispatcher = server.getDispatcher();
  QueryService* service = new QueryServiceImpl();
  
  DebugLog << "================";
  dispatcher->registerService(service);

  server.start();
  return 0;
}
