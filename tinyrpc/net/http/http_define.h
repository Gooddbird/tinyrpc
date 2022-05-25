#ifndef TINYRPC_HTTP_HTTP_DEFINE_H
#define TINYRPC_HTTP_HTTP_DEFINE_H

#include <string>
#include <map>

namespace tinyrpc {

extern std::string g_CRLF;
extern std::string g_CRLF_DOUBLE;

extern std::string content_type_text;
extern const char* default_html_template;

enum HttpMethod {
  GET = 1,
  POST = 2, 
};

enum HttpCode {
  HTTP_OK = 200,
  HTTP_BADREQUEST = 400,
  HTTP_FORBIDDEN = 403,
  HTTP_NOTFOUND = 404,
  HTTP_INTERNALSERVERERROR = 500,
};

const char* httpCodeToString(const int code);

class HttpHeaderComm {
 public:
  HttpHeaderComm() = default;

  virtual ~HttpHeaderComm() = default;

  int getHeaderTotalLength();

  // virtual void storeToMap() = 0;

  std::string getValue(const std::string& key);

  void setKeyValue(const std::string& key, const std::string& value);

  std::string toHttpString();


 public:
  std::map<std::string, std::string> m_maps;

};

class HttpRequestHeader : public HttpHeaderComm {
 public:
  // std::string m_accept;
  // std::string m_referer;
  // std::string m_accept_language;
  // std::string m_accept_charset;
  // std::string m_user_agent; 
  // std::string m_host;
  // std::string m_content_type;
  // std::string m_content_length;
  // std::string m_connection;
  // std::string m_cookie;

//  public:
//   void storeToMap();
};

class HttpResponseHeader : public HttpHeaderComm {
//  public:
//   std::string m_server;
//   std::string m_content_type;
//   std::string m_content_length;
//   std::string m_set_cookie;
//   std::string m_connection;

//  public:
//   void storeToMap(); 
};


}

#endif