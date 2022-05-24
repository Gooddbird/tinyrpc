#include <google/protobuf/service.h>
#include <memory>
#include "tinyrpc/net/http/http_dispatcher.h"
#include "tinyrpc/net/http/http_request.h"
#include "tinyrpc/net/http/http_servlet.h"


namespace tinyrpc {

void HttpDispacther::dispatch(AbstractData* data, TcpConnection* conn) {
  HttpRequest* resquest = dynamic_cast<HttpRequest*>(data);
  HttpResponse response;
  NotFoundHttpServlet servlet(resquest, &response);
  servlet.handle();

  conn->getCodec()->encode(conn->getOutBuffer(), &response);

}

void HttpDispacther::registerServlet(const std::string, HttpServlet::ptr servlet) {

}




}