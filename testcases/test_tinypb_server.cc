#include <google/protobuf/service.h>
#include <sstream>
#include <atomic>

#include "tinyrpc/net/tcp/tcp_server.h"
#include "tinyrpc/net/net_address.h"
#include "tinyrpc/net/mutex.h"
#include "tinyrpc/net/tinypb/tinypb_rpc_dispatcher.h"
#include "tinyrpc/comm/log.h"
#include "tinyrpc/comm/start.h"
#include "test_tinypb_server.pb.h"


static int i = 0;
tinyrpc::CoroutineMutex g_cor_mutex;

class QueryServiceImpl : public QueryService {
 public:
  QueryServiceImpl() {}
  ~QueryServiceImpl() {}

  void query_name(google::protobuf::RpcController* controller,
                       const ::queryNameReq* request,
                       ::queryNameRes* response,
                       ::google::protobuf::Closure* done) {
    
    AppInfoLog << "QueryServiceImpl.query_name, req={"<< request->ShortDebugString() << "}";

    // DebugLog << "========================";
    // DebugLog << "this is query_name func";
    // DebugLog << "first begin to sleep 6s";
    // sleep(6);
    // DebugLog << "sleep 6s end";

    response->set_id(request->id());
    response->set_name("ikerli");

    AppInfoLog << "QueryServiceImpl.query_name, req={"<< request->ShortDebugString() << "}, res={" << response->ShortDebugString() << "}";

    if (done) {
      done->Run();
    }

  }

  void query_age(google::protobuf::RpcController* controller,
                       const ::queryAgeReq* request,
                       ::queryAgeRes* response,
                       ::google::protobuf::Closure* done) {

    AppInfoLog << "QueryServiceImpl.query_age, req={"<< request->ShortDebugString() << "}";
    // AppInfoLog << "QueryServiceImpl.query_age, sleep 6 s begin";
    // sleep(6);
    // AppInfoLog << "QueryServiceImpl.query_age, sleep 6 s end";

    response->set_ret_code(0);
    response->set_res_info("OK");
    response->set_req_no(request->req_no());
    response->set_id(request->id());
    response->set_age(100100111);

    g_cor_mutex.lock();
    AppDebugLog << "begin i = " << i;
    sleep(1);
    i++;
    AppDebugLog << "end i = " << i;
    g_cor_mutex.unlock();

    if (done) {
      done->Run();
    }
    // printf("response = %s\n", response->ShortDebugString().c_str());

    AppInfoLog << "QueryServiceImpl.query_age, res={"<< response->ShortDebugString() << "}";

  }

};


int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("Start TinyRPC server error, input argc is not 2!");
    printf("Start TinyRPC server like this: \n");
    printf("./server a.xml\n");
    return 0;
  }

  tinyrpc::InitConfig(argv[1]);

  REGISTER_SERVICE(QueryServiceImpl);

  tinyrpc::StartRpcServer();
  
  return 0;
}
