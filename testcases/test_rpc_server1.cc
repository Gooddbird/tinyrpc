#include <google/protobuf/service.h>
#include "tcp_server.h"
#include "net_address.h"
#include "tinypb_rpc_dispatcher.h"
#include "log.h"
#include "config.h"
#include "tinypb.pb.h"
#include "coroutine_hook.h"
#include "config.h"


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
    // DebugLog << "first begin to sleep 6s";
    // sleep_hook(6);
    // DebugLog << "sleep 6s end";

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

tinyrpc::Logger* gRpcLogger = nullptr; 
tinyrpc::Config* gRpcConfig = nullptr;


int main(int argc, char* argv[]) {

  gRpcLogger = new tinyrpc::Logger();
  gRpcLogger->init("test_rpc_server1");
  gRpcConfig = new tinyrpc::Config("../testcases/tinyrpc.xml");
  gRpcConfig->readConf();

  tinyrpc::IPAddress::ptr addr = std::make_shared<tinyrpc::IPAddress>("127.0.0.1", 39999);
  
  tinyrpc::TcpServer server(addr);
  tinyrpc::TinyPbRpcDispacther* dispatcher = server.getDispatcher();
  QueryService* service = new QueryServiceImpl();
  
  DebugLog << "================";
  dispatcher->registerService(service);

  server.start();
  return 0;
}
