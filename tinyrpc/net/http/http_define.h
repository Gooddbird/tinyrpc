#ifndef TINYRPC_HTTP_HTTP_DEFINE_H
#define TINYRPC_HTTP_HTTP_DEFINE_H

#include <string>
namespace tinyrpc {

std::string g_CRLF = "\r\n";
std::string g_CRLF_DOUBLE = "\r\n\r\n";

std::string content_type_text = "text/html;charset=utf-8";

const char* default_html_template = "<html><body><h1>%s</h1><p>%s</p></body></html>"; 

enum HttpMethod {
  GET = 1,
  POST = 2, 
};


enum HttpCode {
  HttpOk = 200,
  HttpBadRequset = 400,
  HttpForbidden = 403,
  HttpNotFound = 404,
  HttpInternalServerError = 500,
};

const char* httpCodeToString(const int code) {
  switch (code)
  {
  case HttpOk: 
    return "OK"; 
  
  case HttpBadRequset:
    return "Bad Request";
  
  case HttpForbidden:
    return "Forbidden";
  
  case HttpNotFound:
    return "Not Found";
  
  case HttpInternalServerError:
    return "Internal Server Error";
  
  default:
    return "UnKnown code";
  }

}

class HttpRequestHeader {
 public:
  std::string m_accept;
  std::string m_referer;
  std::string m_accept_language;
  std::string m_user_agent; 
  std::string m_content_type;
  std::string m_host;
  std::string m_content_length;
  std::string m_connection;
};

class HttpResponseHeader {
 public:
  std::string m_server;
  std::string m_content_type;
  std::string m_content_length;
  std::string m_set_cookie;

};


}

#endif