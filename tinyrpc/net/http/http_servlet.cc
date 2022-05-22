#include <memory>
#include "tinyrpc/net/http/http_servlet.h"
#include "tinyrpc/net/http/http_request.h"
#include "tinyrpc/net/http/http_response.h"
#include "tinyrpc/net/http/http_define.h"
#include "tinyrpc/comm/log.h"

namespace tinyrpc {

HttpServlet::HttpServlet(HttpRequest::ptr req, HttpResponse::ptr res) : m_request(req), m_response(res) {
  setCommParam();
}

HttpServlet::~HttpServlet() {

}

void HttpServlet::handle() {


}

void HttpServlet::handleNotFound() {
  DebugLog << "return 404 html";
  setHttpCode(HttpNotFound);
  char buf[512];
  sprintf(buf, default_html_template, std::to_string(HttpNotFound).c_str(), httpCodeToString(HttpNotFound));
  m_response->m_response_body = std::string(buf);
  m_response->m_response_header.m_content_type = content_type_text;
  m_response->m_response_header.m_content_length = m_response->m_response_body.length();
}

void HttpServlet::setHttpCode(const int code) {
  m_response->m_response_code = code;
  m_response->m_response_info = std::string(httpCodeToString(code));
}

void HttpServlet::setCommParam() {
  m_response->m_response_version = m_request->m_request_version;
}

}



