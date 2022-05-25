#include <google/protobuf/service.h>
#include <memory>
#include "tinyrpc/net/http/http_dispatcher.h"
#include "tinyrpc/net/http/http_request.h"
#include "tinyrpc/net/http/http_servlet.h"
#include "tinyrpc/comm/log.h"


namespace tinyrpc {

void HttpDispacther::dispatch(AbstractData* data, TcpConnection* conn) {
  HttpRequest* resquest = dynamic_cast<HttpRequest*>(data);
  HttpResponse response;
  std::string url_path = resquest->m_request_path;
  if (!url_path.empty()) {
    auto it = m_servlets.find(url_path);
    if (it == m_servlets.end()) {
      ErrorLog << "404, url path{ " << url_path << "} not register servlet";
      NotFoundHttpServlet servlet;
      servlet.setCommParam(resquest, &response);
      servlet.handle(resquest, &response);
    } else {
      it->second->setCommParam(resquest, &response);
      it->second->handle(resquest, &response);
    }
  }

  conn->getCodec()->encode(conn->getOutBuffer(), &response);

}

void HttpDispacther::registerServlet(const std::string& path, HttpServlet::ptr servlet) {
  auto it = m_servlets.find(path);
  if (it == m_servlets.end()) {
    DebugLog << "register servlet success to path {" << path << "}";
    m_servlets[path] = servlet;
  } else {
    ErrorLog << "failed to register, beacuse path {" << path << "} has already register sertlet";
  }
}




}