## This is an example that how to make a simple rpc testcase with "tinyrpc"

#### 1. Define protobuf file

A simple **protobuf** file like this:

```c++
// testcases/tinypb.proto
syntax = "proto3";
option cc_generic_services = true;

message queryAgeReq {
  int32 req_no = 1;
  int32 id = 2;
}

message queryAgeRes {
  int32 ret_code = 1;
  string res_info = 2;
  int32 req_no = 3;
  int32 id = 4;
  int32 age = 5;
}

message queryNameReq {
  int32 req_no = 1;
  int32 id = 2;
  int32 type = 3;
}

message queryNameRes {
  int32 ret_code = 1;
  string res_info = 2;
  int32 req_no = 3;
  int32 id = 4;
  string name = 5;
}


service QueryService {
  // rpc method name
  rpc query_name(queryNameReq) returns (queryNameRes);

  // rpc method name
  rpc query_age(queryAgeReq) returns (queryAgeRes);
}
```

#### 2. Use protoc to auto-gen code
```
protoc --cpp_out=./ tinypb.proto
```
And you will get two files:

**tinypb.pb.cc**   **tinypb.pb.h**

It contains main class, for example: **queryAgeReq、QueryService、QueryService_Stub**


#### 3. Implementation abstract interface "QueryService"
You should inherit class "**QueryService**" and write business code to finish method "**query_name**" and "**query_age**", because these methods is vritual function in **QueryService**.

```c++
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

    response->set_ret_code(0);
    response->set_res_info("OK");

    tinyrpc::MySQLInstase* instase =  tinyrpc::MySQLInstaseFactroy::GetThreadMySQLFactory()->GetMySQLInstase("test_db_key1");
    if (!instase->isInitSuccess()) {
      ErrorLog << "mysql instase init failed";
      return;
    }

    char query_sql[512];
    sprintf(query_sql, "select user_id, user_name, user_gender from user_db.t_user_information where user_id = '%s';", std::to_string(request->id()).c_str());

    int rt = instase->query(std::string(query_sql));
    if (rt != 0) {
      ErrorLog << "query return not 0";
      return;
    }
    MYSQL_RES* res = instase->storeResult();

    MYSQL_ROW row = instase->fetchRow(res);
    if (row) {
      int i = 0;
      response->set_id(std::atoi(row[i++]));
      response->set_name(std::string(row[i++]));
    } else {
      DebugLog << "query empty";
      response->set_ret_code(999);
      response->set_res_info("this user not exist");
    }

    
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
```

#### 4. Create XML config file
for example:
```xml
<?xml version="1.0" encoding="UTF-8" ?>
<root>
  <!--log config-->
  <log>
    <!--identify path of log file-->
    <log_path>./</log_path>
    <log_prefix>test_rpc_server1</log_prefix>

    <!--identify max size of single log file, MB-->
    <log_max_file_size>5</log_max_file_size>

    <!--log level: DEBUG < INFO < WARN < ERROR-->
    <log_level>DEBUG</log_level>

    <!--inteval that put log info to async logger, s-->
    <log_sync_inteval>1</log_sync_inteval>
  </log>

  <coroutine>
    <!--coroutine stack size (KB)-->
    <coroutine_stack_size>128</coroutine_stack_size>

    <!--default coroutine pool size-->
    <coroutine_pool_size>5000</coroutine_pool_size>

  </coroutine>

  <msg_req_len>20</msg_req_len>

  <!--max time when call connect, s-->
  <max_connect_timeout>75</max_connect_timeout>

  <!--count of io threads, at least 1-->
  <iothread_num>2</iothread_num>

  <time_wheel>
    <bucket_num>6</bucket_num>

    <!--inteval that destroy bad TcpConnection, s-->
    <inteval>10</inteval>
  </time_wheel>


  <database>
    <db_key name="test_db_key1">
      <!-- <ip>127.0.0.1</ip> -->
      <ip>192.168.245.7</ip>
      <port>3306</port>
      <user>root</user>
      <passwd>Ikerli20220517!!</passwd>
      <select_db></select_db>
      <char_set>utf8mb4</char_set>
    </db_key>
  </database>

</root>
```

#### 5. Set Up RpcServer1
It's very convenient to set uo RpcServer with **tinyrpc** framework

```c++

// testcases/test_rpc_server1.cc
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
```

#### 6.Set Up RpcServer2 
Like RpcServer1, but we don't register Service, because we just user RpcServer2 to call method of RpcServer1.
```c++
// testcases/test_rpc_server2.cc
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


tinyrpc::Config::ptr gRpcConfig;

int main(int argc, char* argv[]) {

  if (argc != 3) {
    std::cout << "use example:  ./out [port] [num]" << std::endl;
    std::cout << "./out 30001 1" << std::endl;
    return 0;
  }

  gRpcConfig = std::make_shared<tinyrpc::Config>("../testcases/test_rpc_server2.xml");
  gRpcConfig->readConf();

  int port = std::atoi(argv[1]);
  n = std::atoi(argv[2]);

  tinyrpc::IPAddress::ptr self_addr = std::make_shared<tinyrpc::IPAddress>("127.0.0.1", port);
  tinyrpc::TcpServer server(self_addr);
  tinyrpc::Coroutine::ptr cor = tinyrpc::GetCoroutinePool()->getCoroutineInstanse();
  cor->setCallBack(&fun);
  server.addCoroutine(cor);

  server.start();

  return 0;
}
```

#### 7. Test Rpc
Directly run RpcServer1 and RpcServer2, notice if you recive correct result;
You can see more detail in log file. like '**test_rpc_server1_20220516_rpc_0.log**'
