#include <google/protobuf/service.h>
#include "tcp_server.h"
#include "net_address.h"
#include "tinypb_rpc_dispatcher.h"
#include "log.h"
#include "config.h"
#include "tinypb.pb.h"
#include "coroutine_hook.h"
#include "config.h"
#include "tinyrpc/comm/mysql_instase.h"
#include <mysql/mysql.h>
#include <sstream>

tinyrpc::Config::ptr gRpcConfig;

class QueryServiceImpl : public QueryService {
 public:
  QueryServiceImpl() {}
  ~QueryServiceImpl() {}

  void query_name(google::protobuf::RpcController* controller,
                       const ::queryNameReq* request,
                       ::queryNameRes* response,
                       ::google::protobuf::Closure* done) {
    
    // DebugLog << "========================";
    // DebugLog << "this is query_name func";
    // DebugLog << "first begin to sleep 6s";
    // sleep_hook(6);
    // DebugLog << "sleep 6s end";
    tinyrpc::MySQLInstase* instase =  tinyrpc::MySQLInstaseFactroy::GetThreadMySQLFactory()->GetMySQLInstase("test_db_key1");
    if (!instase->isInitSuccess()) {
      ErrorLog << "mysql instase init failed";
      return;
    }

    char query_sql[512];
    sprintf(query_sql, "select user_id, user_name, user_gender from user_db.t_user_information where user_id = '%s';", std::to_string(request->id()).c_str());

    int rt = instase->Query(std::string(query_sql));
    if (rt != 0) {
      ErrorLog << "query return not 0";
      return;
    }
    MYSQL_RES* res = instase->StoreResult();

    MYSQL_ROW row = instase->FetchRow(res);
    if (!row) {
      int i = 0;
      response->set_id(std::atoi(row[i++]));
      response->set_name(std::string(row[i++]));

    }

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

  gRpcConfig = std::make_shared<tinyrpc::Config>("../testcases/test_rpc_server1.xml");
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
