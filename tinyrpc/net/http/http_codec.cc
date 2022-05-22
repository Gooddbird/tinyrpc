#include <algorithm>
#include "http_codec.h"
#include "tinyrpc/comm/log.h"
#include "tinyrpc/net/abstract_data.h"
#include "tinyrpc/net/abstract_codec.h"
#include "tinyrpc/net/http/http_request.h"

namespace tinyrpc {

extern std::string g_CRLF = "\r\n";
extern std::string g_CRLF_DOUBLE = "\r\n\r\n";

HttpCodeC::HttpCodeC() {

}

HttpCodeC::~HttpCodeC() {

}

void HttpCodeC::encode(TcpBuffer* buf, AbstractData* data) {
  DebugLog << "test encode";
  buf->readAble();
}

void HttpCodeC::decode(TcpBuffer* buf, AbstractData* data) {
  DebugLog << "test http decode start";
  m_strs = "";
  if (!buf || !data) {
    ErrorLog << "decode error! buf or data nullptr";
    return;
  }
  HttpRequest* request = dynamic_cast<HttpRequest*>(data);

  m_strs = buf->getBufferString();
  int start_index = buf->readIndex();
  int end_index = -1;

  bool is_parse_request_line = false;
  bool is_parse_request_header = false;
  bool is_parse_request_content = false;
  bool is_parse_succ = false;
  std::string tmp(m_strs);
  int len = tmp.length();
  while (1) {
    size_t i = tmp.find(g_CRLF);
    if (i == tmp.npos) {
      DebugLog << "not found CRLF in buffer";
      break;
    }
    if (i == tmp.length() - 2) {
      DebugLog << "need to read more data";
      break;
    }
    is_parse_request_line = parseHttpRequestLine(request, i);
    if (!is_parse_request_line) {
      break;
    }
    tmp = tmp.substr(i + 2, len - 3 - i);
  }

  DebugLog << "test decode end";
}


bool HttpCodeC::parseHttpRequestLine(HttpRequest* requset, int i) {
  std::string tmp = m_strs.substr(0, i);
  size_t s1 = tmp.find_first_of(" ");
  size_t s2 = tmp.find_last_of(" ");

  if (s1 == tmp.npos || s2 == tmp.npos || s1 == s2) {
    ErrorLog << "error read Http Requser Line, space is not 2";
    return false;
  }
  std::string method = tmp.substr(0, s1);
  std::transform(method.begin(), method.end(), method.begin(), toupper);
  if (method == "GET") {
    requset->m_request_method = HttpMethod::GET;
  } else if (method == "POST") {
    requset->m_request_method = HttpMethod::POST;
  } else {
    ErrorLog << "parse http request request line error, not support http method:" << method;
    return false;
  }

  std::string version = tmp.substr(s2 + 1, tmp.length() - s2 - 2);
  std::transform(version.begin(), version.end(), version.begin(), toupper);
  if (version != "HTTP/1.1" && version != "HTTP/1.0") {
    ErrorLog << "parse http request request line error, not support http version:" << version;
    return false;
  }
  

  std::string url = tmp.substr(s1 + 1, s2 - s1 - 1);
  size_t j = url.find("://");

  if (j == url.npos || j + 3 >= url.length()) {
    ErrorLog << "parse http request request line error, bad url:" << url;
    return false;
  }
  url = url.substr(j + 3, s2 - s1  - j - 4);
  j = url.find_first_of("/");
  int l = url.length();
  if (j == url.npos || j == url.length() - 1) {
    DebugLog << "http request root path, and query is empty";
    return true;
  }
  url = url.substr(j + 1, l - j - 2);
  l = url.length();
  j = url.find_first_of("?");
  requset->m_request_path = url.substr(0, j);
  if (j == url.npos) {
    DebugLog << "http request path:" << requset->m_request_path << "and query is empty";
    return true;
  }
  requset->m_request_query = url.substr(j + 1, l - j - 2);
  DebugLog << "http request path:" << requset->m_request_path << "and query:" << requset->m_request_query;
  return true;

}

bool HttpCodeC::parseHttpRequestHeader(HttpRequest* requset, int i) {
  return true;
}

}
