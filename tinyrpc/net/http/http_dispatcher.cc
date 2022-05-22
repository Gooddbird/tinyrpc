#include <google/protobuf/service.h>
#include "tinyrpc/net/http/http_dispatcher.h"
#include "tinyrpc/net/http/http_request.h"
#include "tinyrpc/net/http/http_servlet.h"


namespace tinyrpc {

void HttpDispacther::dispatch(AbstractData* data, TcpConnection* conn) {

}

void HttpDispacther::registerServlet(const std::string, HttpServlet::ptr servlet) {

}




}