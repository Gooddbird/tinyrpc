#include <google/protobuf/service.h>
#include "tcp_server.h"
#include "net_address.h"
#include "tinypb_rpc_dispatcher.h"
#include "log.h"
#include "tinyrpc/comm/start.h"
#include "tinypb.pb.h"
#include "coroutine_hook.h"
#include "tinyrpc/comm/start.h"
#include "tinyrpc/comm/mysql_instase.h"
#include "tinyrpc/comm/start.h"
#include <mysql/mysql.h>
#include <sstream>
#include "tinyrpc/net/http/http_request.h"
#include "tinyrpc/net/http/http_response.h"
#include "tinyrpc/net/http/http_servlet.h"
#include "tinyrpc/net/http/http_define.h"

class RootHttpServlet : public tinyrpc::HttpServlet {
 public:
  RootHttpServlet() = default;
  ~RootHttpServlet() = default;

  void handle(tinyrpc::HttpRequest* req, tinyrpc::HttpResponse* res) {
    DebugLog << "success recive http request, now to get http response";
    setHttpCode(res, tinyrpc::HTTP_OK);
    setHttpBody(res, "<html><body><h1>Welcome to TINYRPC, just enjoy it!</h1><p>Thanks!</p></body></html>");
    setHttpContentType(res, "text/html;charset=utf-8");
    DebugLog << "get http response success";
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

  RootHttpServlet::ptr rootservlet = std::make_shared<RootHttpServlet>();
  tinyrpc::InitConfig(argv[1]);
  tinyrpc::GetServer()->registerHttpServlet("/", rootservlet);

  tinyrpc::StartRpcServer();

  
  return 0;
}
