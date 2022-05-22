#ifndef TINYRPC_NET_HTTP_HTTP_SERVLET_H
#define TINYRPC_NET_HTTP_HTTP_SERVLET_H

#include <memory>
#include "tinyrpc/net/http/http_request.h"
#include "tinyrpc/net/http/http_response.h"

namespace tinyrpc {

class HttpServlet {
 public:
  typedef std::shared_ptr<HttpServlet> ptr;

  HttpServlet(HttpRequest::ptr req, HttpResponse::ptr res);

  virtual ~HttpServlet();

  virtual void handle() = 0;

  void handleNotFound();

  void setHttpCode(const int code);

 private:
  void setCommParam();

 private:
  HttpRequest::ptr m_request;
  HttpResponse::ptr m_response;

};

}


#endif
