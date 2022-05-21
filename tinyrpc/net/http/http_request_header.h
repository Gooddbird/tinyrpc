#ifndef TINYRPC_HTTP_HTTP_REQUEST_HEADER_H
#define TINYRPC_HTTP_HTTP_REQUEST_HEADER_H

#include <string>

namespace tinyrpc {

class HttpRequestHeader {
 public:
  std::string m_accrpt;
  std::string m_referer;
  std::string m_accept_language;
  std::string m_user_agent; 
  std::string m_content_type;
  std::string m_host;
  std::string m_content_length;
  std::string m_connection;
};


}

#endif