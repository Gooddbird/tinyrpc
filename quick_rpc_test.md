# Quick Start of TinyRPC
使用 TinyRPC 框架，可以很方便、快捷的就搭建一个 RPC 服务，简单到只需要几行代码就行了！
以下给出两个 RPC 服务的例子，其中一个使用标准的 HTTP 协议的 RPC 服务A，另一个使用 TinyPB 协议的服务B。
然后通过浏览器发起 HTTP 请求到 服务A，在由服务 A 调用 服务 B 查询数据，最后返回给浏览器前端页面。

## 1. 基于 TinyPB 协议的 RPC 服务搭建

### 1.1 编写一个 PortoBuf 文件
TinyPB 协议是基于 ProtoBuf 库来序列化数据的，因此第一步要编写一个 protobuf 文件。
ProtoBuf 的语法很简单，就像定义结构体一样罢了。例如：
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

### 1.2 使用 protoc 生成代码
没什么好说的，感谢 google 的 ProtoBuf.

```
protoc --cpp_out=./ tinypb.proto
```
执行完后可以发现当前路径下新增了两个文件：

**tinypb.pb.cc**   **tinypb.pb.h**

这两个文件包含了一些 RPC 主要类，如: **queryAgeReq、QueryService、QueryService_Stub**

### 1.3 继承类 "QueryService"

QueryService 是个抽象基类，因此必须要继承然后重写 "**query_name**" and "**query_age**" 这两个方法才行。这个时候仅仅需要在这两个方法里面实现业务逻辑就行了。
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
      response->set_ret_code(-1);
      response->set_res_info("faild to init mysql");
      ErrorLog << "mysql instase init failed";
      return;
    }

    char query_sql[512];
    sprintf(query_sql, "select user_id, user_name, user_gender from user_db.t_user_information where user_id = '%s';", std::to_string(request->id()).c_str());

    int rt = instase->query(std::string(query_sql));
    if (rt != 0) {
      response->set_ret_code(-1);
      response->set_res_info(instase->getMySQLErrorInfo());
      ErrorLog << "failed to excute sql" << std::string(query_sql);
      return;
    }

    MYSQL_RES* res = instase->storeResult();

    MYSQL_ROW row = instase->fetchRow(res);
    if (row) {
      int i = 0;
      DebugLog << "query success";
      response->set_id(std::atoi(row[i++]));
      response->set_name(std::string(row[i++]));
    } else {
      DebugLog << "query empty";
      response->set_ret_code(-1);
      response->set_res_info("this user not exist");
    }
    if (done) {
      done->Run();
    }
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

这里很简单，就是根据 request 的参数去数据库里面查询用户信息，然后返回到 response 对象。

此外，还需要先建 MySQL 对应库表才行。连接上 MySQL 后，执行以下语句：
```sql
# 先建立数据库
CREATE DATABASE user_db;

# 然后建表
CREATE TABLE `t_user_information` (
  `user_id` varchar(20) NOT NULL DEFAULT '',
  `user_name` varchar(200) NOT NULL DEFAULT '',
  `user_gender` smallint(10) NOT NULL DEFAULT '0',
  `modify_time` datetime NOT NULL,
  `create_time` datetime NOT NULL,
  PRIMARY KEY (`user_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

# 插入一条测试数据
INSERT INTO t_user_information VALUE ('1100110001', 'ikerli', 1, now(), now());

```

### 1.4 建立配置文件
配置文件是必须的，TinyRPC 框架开始运行时会从配置文件里面读取相应的必要数据。这里不一一细说各个配置的作用，更多详细信息见配置文件说明文档。
一个完整的配置文件如下：

```xml
<?xml version="1.0" encoding="UTF-8" ?>
<root>
  <!--log config-->
  <log>
    <!--日志文件路径，这个是相对运行时候 server 所在文件的路径-->
    <log_path>./</log_path>
    <!--日志文件前缀，一般跟 服务的名字相同-->
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

  <server>
    <!--服务器绑定的 ip-->
    <ip>192.168.245.7</ip>
    <!--服务器监听的端口-->
    <port>39999</port>

    <!--服务器协议-->
    <protocal>TinyPB</protocal>
  </server>

  <database>
    <!--这里需要换成自己的 MySQL 配置信息-->
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

这里配置基本没什么改的，注意下日志文件名，以及 ip 和 port 配置就好了。此外，由于这里使用了MySQL, 还需要配置自己要连接的 MySQL 数据库的配置。

注意这里 protocal 字段要选择 'TinyPB' 协议。

### 1.5 实现 RPC 服务

使用 TinyRPC 框架搭建一个 RPC 服务非常快捷，只需要这寥寥几行代码即可。

```c++
int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("Start TinyRPC server error, input argc is not 2!");
    printf("Start TinyRPC server like this: \n");
    printf("./server a.xml\n");
    return 0;
  }

  tinyrpc::InitConfig(argv[1]);

  tinyrpc::GetServer()->registerService(std::make_shared<QueryServiceImpl>());

  tinyrpc::StartRpcServer();
  
  return 0;
}
```
注意，这里 argv[1] 参数是配置文件的路径。

registerService 代表将 Service 注册到 RPC 服务上，这一步也是必须的。

然后就可以启动 RPC 服务了，推荐使用守护进程方式启动。命令如下：

```
nohup ./test_rpc_server1 ../conf/test_rpc_server1.xml &
```
如果没什么报错信息，那么恭喜你启动成功了。如果不放心，可以使用 ps 命令查看进程是否存在：

```
ps -elf | grep 'test_rpc_server1'
```

然后你可以观察到日志路径下会输出一些日志文件，通过日志文件能看到更多详细的信息。

至此，基于 TinyPB 协议的 RPC 服务就搭建好了。

## 2. 基于 HTTP 协议的 RPC 服务搭建
### 2.1 配置文件
配置文件基本跟上面的一致，只需要注意修改几个地方：
- port: 端口号需要重新选择一个，否则会跟前面的服务冲突。这里我选择 19998.
- protocal: 协议需要选择 'HTTP'。

我的配置文件如下:
```
<?xml version="0.0" encoding="UTF-8" ?>
<root>
  <!--log config-->
  <log>
    <!--identify path of log file-->
    <log_path>./</log_path>
    <log_prefix>http_server</log_prefix>

    <!--identify max size of single log file, MB-->
    <log_max_file_size>4</log_max_file_size>

    <!--log level: DEBUG < INFO < WARN < ERROR-->
    <log_level>DEBUG</log_level>

    <!--inteval that put log info to async logger, s-->
    <log_sync_inteval>0</log_sync_inteval>
  </log>

  <coroutine>
    <!--coroutine stack size (KB)-->
    <coroutine_stack_size>127</coroutine_stack_size>

    <!--default coroutine pool size-->
    <coroutine_pool_size>4999</coroutine_pool_size>

  </coroutine>

  <msg_req_len>19</msg_req_len>

  <!--max time when call connect, s-->
  <max_connect_timeout>74</max_connect_timeout>

  <!--count of io threads, at least 0-->
  <iothread_num>1</iothread_num>

  <time_wheel>
    <bucket_num>5</bucket_num>

    <!--inteval that destroy bad TcpConnection, s-->
    <inteval>9</inteval>
  </time_wheel>

  <server>
    <ip>191.168.245.7</ip>
    <port>19998</port>
    <protocal>HTTP</protocal>
  </server>

</root>
```
### 2.2 实现 Servlet
TinyRPC 框架模拟了 Java 的 Servlet 来提供 Web 服务。使用非常简单，只需要继承虚拟的 HttpServlet 类并实现其虚函数 handle 方法即可。

```c++
const char* html = "<html><body><h1>Welcome to TINYRPC, just enjoy it!</h1><p>%s</p></body></html>";

class RootHttpServlet : public tinyrpc::HttpServlet {
 public:
  RootHttpServlet() = default;
  ~RootHttpServlet() = default;

  void handle(tinyrpc::HttpRequest* req, tinyrpc::HttpResponse* res) {
    DebugLog << "success recive http request, now to get http response";
    setHttpCode(res, tinyrpc::HTTP_OK);
    setHttpContentType(res, "text/html;charset=utf-8");


    queryNameReq rpc_req;
    queryNameRes rpc_res;
    DebugLog << "now to call QueryServer TinyRPC server to query who's id is " << req->m_query_maps["id"];
    rpc_req.set_id(std::atoi(req->m_query_maps["id"].c_str()));

    tinyrpc::TinyPbRpcChannel channel(std::make_shared<tinyrpc::IPAddress>("127.0.0.1", 39999));
    QueryService_Stub stub(&channel);

    tinyrpc::TinyPbRpcController rpc_controller;
    rpc_controller.SetTimeout(5000);

    stub.query_name(&rpc_controller, &rpc_req, &rpc_res, NULL);

    if (rpc_controller.ErrorCode() != 0) {
      ErrorLog << "failed to call QueryServer rpc server";
      char buf[512];
      sprintf(buf, html, "failed to call QueryServer rpc server");
      setHttpBody(res, std::string(buf));
      return;
    }

    if (rpc_res.ret_code() != 0) {
      std::stringstream ss;
      ss << "QueryServer rpc server return bad result, ret = " << rpc_res.ret_code() << ", and res_info = " << rpc_res.res_info();
      ErrorLog << ss.str();
      char buf[512];
      sprintf(buf, html, ss.str().c_str());
      setHttpBody(res, std::string(buf));
      return;
    }

    std::stringstream ss;
    ss << "Success!! Your name is " << rpc_res.name() << ", and Your id is " << rpc_res.id();

    char buf[512];
    sprintf(buf, html, ss.str().c_str());
    setHttpBody(res, std::string(buf));

  }

};
```
可以看到，这里实现很简单。 只是根据 Http 请求里面的 id 字段，然后去调用 QueryServer 服务查询对应 id 的用户信息，然后组成 HTML 格式返回给前端。

注意这里判断 RPC 调用结果有两步，先判断了 RpcControll 的 ErrorCode 是否为0，若不为零代表 RPC 调用出现了框架级错误，可能造成原因是 connect 失败、超时等等。

然后再次判断了 rpc_res.res_code 是否为0，这个错误代码是业务级别的错误码，说明对端业务处理出现了逻辑错误，但此次 RPC 调用是成功的。


### 2.3 搭建 RPC 服务
很简单，几行代码就足够了：

```c++
int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("Start TinyRPC server error, input argc is not 2!");
    printf("Start TinyRPC server like this: \n");
    printf("./server a.xml\n");
    return 0;
  }

  tinyrpc::InitConfig(argv[1]);

  tinyrpc::GetServer()->registerHttpServlet("/user", std::make_shared<RootHttpServlet>());

  tinyrpc::StartRpcServer();

  return 0;
}
```

这里需要注意 registerHttpServlet 这里。这是将自己实现的 Servlet 注册到 RPC 服务中。这里的 "/user" 参数代表 url 路径。例如，完整的 url 就是：

```
http://192.168.245.7:19999/user
```

代表当访问这个 url 地址时， RPC 服务便会找到 RootHttpServlet 这个 Servlet，然后调用其 handle 方法进行处理。

随后启动这个服务就好了：

```
nohup ./test_http_server ../conf/test_http_server.xml &
```
至此，基于 HTTP 协议的 RPC 服务也搭建好了。接下来做个验证。


## 3. 简单测试

测试很简单，打开浏览器，键入以下 url 直接访问即可。
```
http://192.168.245.7:19999/user?id=1100110001
```
注意：如果浏览器跟服务不再同一台机器上，需要注意是否有防火墙等拦截。如 centos 关闭防火墙可以使用以下命令：
```
sudo systemctl stop firewalld.service
```

当然，也可以直接使用 curl 工具发送 Http 协议来测试：
```
curl -X GET "http://192.168.245.7:19999/user?id=1100110001"
```

如果测试正常，你将得到以下页面：


![](imgs/test_succ.png)
