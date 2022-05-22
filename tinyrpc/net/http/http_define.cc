#include <string>
#include "tinyrpc/net/http/http_define.h"

namespace tinyrpc {

std::string g_CRLF = "\r\n";
std::string g_CRLF_DOUBLE = "\r\n\r\n";

std::string content_type_text = "text/html;charset=utf-8";
const char* default_html_template = "<html><body><h1>%s</h1><p>%s</p></body></html>";


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

}