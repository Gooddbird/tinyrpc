#include <google/protobuf/service.h>
#include "tinypb.pb.h"
#include "tinyrpc/comm/start.h"
#include "tinyrpc/net/http/http_request.h"
#include "tinyrpc/net/http/http_response.h"
#include "tinyrpc/net/http/http_servlet.h"
#include "tinyrpc/net/http/http_define.h"
#include "tinyrpc/net/tinypb/tinypb_rpc_channel.h"
#include "tinyrpc/net/tinypb/tinypb_rpc_async_channel.h"
#include "tinyrpc/net/tinypb/tinypb_rpc_controller.h"
#include "tinyrpc/net/net_address.h"
#include <atomic>
#include <future>


const char* html = "<html><body><h1>Welcome to TinyRPC, just enjoy it!</h1><p>%s</p></body></html>";
static std::atomic_int count{1};

// static thread_local int t_index = 1;

class RootHttpServlet : public tinyrpc::HttpServlet {
 public:
  RootHttpServlet() = default;
  ~RootHttpServlet() = default;

  void handle(tinyrpc::HttpRequest* req, tinyrpc::HttpResponse* res) {
    AppDebugLog << "RootHttpServlet get request count =" << count;
    AppDebugLog << "RootHttpServlet success recive http request, now to get http response";
    setHttpCode(res, tinyrpc::HTTP_OK);
    setHttpContentType(res, "text/html;charset=utf-8");


    queryNameReq rpc_req;
    queryNameRes rpc_res;
    AppDebugLog << "now to call QueryServer TinyRPC server to query who's id is " << req->m_query_maps["id"];
    AppDebugLog << "now to call QueryServer TinyRPC server to query who's id is " << req->m_query_maps["id"];
    rpc_req.set_id(std::atoi(req->m_query_maps["id"].c_str()));

    tinyrpc::TinyPbRpcChannel channel(std::make_shared<tinyrpc::IPAddress>("127.0.0.1", 39999));
    QueryService_Stub stub(&channel);

    tinyrpc::TinyPbRpcController rpc_controller;
    rpc_controller.SetTimeout(10000);

    DebugLog << "RootHttpServlet begin to call RPC" << count;
    stub.query_name(&rpc_controller, &rpc_req, &rpc_res, NULL);
    DebugLog << "RootHttpServlet end to call RPC" << count;

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
    ss << "req_count = " << count <<  "Success!! Your name is " << rpc_res.name() << ", and Your id is " << rpc_res.id();

    // std::stringstream ss;
    // ss << "req_count = " << count <<  "Success!! Your name is " << "ikerli " << ", and Your id is " << "1111";

    char buf[512];
    sprintf(buf, html, ss.str().c_str());
    setHttpBody(res, std::string(buf));
    count++;

  }

  std::string getServletName() {
    return "RootHttpServlet";
  }

 private:

};

class AnotherHttpServlet : public tinyrpc::HttpServlet {
 public:
  AnotherHttpServlet() = default;
  ~AnotherHttpServlet() = default;

  void handle(tinyrpc::HttpRequest* req, tinyrpc::HttpResponse* res) {
    DebugLog << "AnotherHttpServlet get request count =" << count;
    DebugLog << "AnotherHttpServlet success recive http request, now to get http response";
    setHttpCode(res, tinyrpc::HTTP_OK);
    setHttpContentType(res, "text/html;charset=utf-8");


    queryAgeReq rpc_req;
    queryAgeRes rpc_res;
    DebugLog << "now to call QueryServer TinyRPC server to query who's id is " << req->m_query_maps["id"];
    rpc_req.set_id(std::atoi(req->m_query_maps["id"].c_str()));

    tinyrpc::TinyPbRpcChannel channel(std::make_shared<tinyrpc::IPAddress>("127.0.0.1", 39999));
    QueryService_Stub stub(&channel);

    tinyrpc::TinyPbRpcController rpc_controller;
    rpc_controller.SetTimeout(5000);

    DebugLog << "AnotherHttpServlet end to call RPC" << count;
    stub.query_age(&rpc_controller, &rpc_req, &rpc_res, NULL);
    DebugLog << "AnotherHttpServlet end to call RPC" << count;

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
    ss << "req_count = " << count <<  "Success!! Your age is," << rpc_res.age() << " and Your id is " << rpc_res.id();

    char buf[512];
    sprintf(buf, html, ss.str().c_str());
    setHttpBody(res, std::string(buf));
    count++;

  }

  std::string getServletName() {
    return "AnotherHttpServlet";
  }

 private:

};

class AsyncRPCTestServlet: public tinyrpc::HttpServlet {
 public:
  AsyncRPCTestServlet() = default;
  ~AsyncRPCTestServlet() = default;

  void handle(tinyrpc::HttpRequest* req, tinyrpc::HttpResponse* res) {
    AppDebugLog << "AnotherHttpServlet get request count =" << count++;
    AppDebugLog << "AnotherHttpServlet success recive http request, now to get http response";
    setHttpCode(res, tinyrpc::HTTP_OK);
    setHttpContentType(res, "text/html;charset=utf-8");


    queryAgeReq rpc_req;
    queryAgeRes rpc_res;
    AppDebugLog << "now to call QueryServer TinyRPC server to query who's id is " << req->m_query_maps["id"];
    rpc_req.set_id(std::atoi(req->m_query_maps["id"].c_str()));

    tinyrpc::TinyPbRpcAsyncChannel::ptr async_channel = std::make_shared<tinyrpc::TinyPbRpcAsyncChannel>(std::make_shared<tinyrpc::IPAddress>("127.0.0.1", 39999));
    QueryService_Stub stub(async_channel.get());

    tinyrpc::TinyPbRpcController rpc_controller;
    rpc_controller.SetTimeout(2000);

    AppDebugLog << "AsyncRPCTestServlet begin to call RPC async";
    stub.query_age(&rpc_controller, &rpc_req, &rpc_res, NULL);
    AppDebugLog << "AsyncRPCTestServlet async end, now you can to some another thing";
    AppDebugLog << "AsyncRPCTestServlet test to sleep 2 s when call async rpc";
    sleep(2);
    AppDebugLog << "AsyncRPCTestServlet test sleep 2 s when call async rpc back, now to call future.wait()";

    async_channel->getFuture().wait();
    AppDebugLog << "future.wait() back, now to check is rpc call succ";

    if (rpc_controller.ErrorCode() != 0) {
      AppDebugLog << "failed to call QueryServer rpc server";
      char buf[512];
      sprintf(buf, html, "failed to call QueryServer rpc server");
      setHttpBody(res, std::string(buf));
      return;
    }

    if (rpc_res.ret_code() != 0) {
      std::stringstream ss;
      ss << "QueryServer rpc server return bad result, ret = " << rpc_res.ret_code() << ", and res_info = " << rpc_res.res_info();
      AppDebugLog << ss.str();
      char buf[512];
      sprintf(buf, html, ss.str().c_str());
      setHttpBody(res, std::string(buf));
      return;
    }

    std::stringstream ss;
    ss << "Success!! Your age is," << rpc_res.age() << " and Your id is " << rpc_res.id();

    char buf[512];
    sprintf(buf, html, ss.str().c_str());
    setHttpBody(res, std::string(buf));
  }

  std::string getServletName() {
    return "AsyncRPCTestServlet";
  }
  

 private:

};




int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("Start TinyRPC server error, input argc is not 2!");
    printf("Start TinyRPC server like this: \n");
    printf("./server a.xml\n");
    return 0;
  }

  tinyrpc::InitConfig(argv[1]);

  tinyrpc::GetServer()->registerHttpServlet("/user", std::make_shared<RootHttpServlet>());
  tinyrpc::GetServer()->registerHttpServlet("/another", std::make_shared<AnotherHttpServlet>());
  tinyrpc::GetServer()->registerHttpServlet("/async", std::make_shared<AsyncRPCTestServlet>());

  tinyrpc::StartRpcServer();

  
  return 0;
}
