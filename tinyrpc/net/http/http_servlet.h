#ifndef TINYRPC_NET_HTTP_HTTP_SERVLET_H
#define TINYRPC_NET_HTTP_HTTP_SERVLET_H

#include <memory>
#include "tinyrpc/net/http/http_request.h"
#include "tinyrpc/net/http/http_response.h"

namespace tinyrpc {

class HttpServlet : public std::enable_shared_from_this<HttpServlet> {
 public:
  typedef std::shared_ptr<HttpServlet> ptr;

  HttpServlet(HttpRequest* req, HttpResponse* res);

  virtual ~HttpServlet();

  virtual void handle() = 0;

  void handleNotFound();

  void setHttpCode(const int code);

 protected:
  void setCommParam();

 protected:
  HttpRequest* m_request;
  HttpResponse* m_response;

};


class NotFoundHttpServlet: public HttpServlet {
 public:

  NotFoundHttpServlet(HttpRequest* req, HttpResponse* res);

  ~NotFoundHttpServlet();

  void handle();

};

}


#endif
