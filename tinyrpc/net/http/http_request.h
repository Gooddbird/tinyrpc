#ifndef TINYRPC_NET_HTTP_HTTP_REQUEST_H
#define TINYRPC_NET_HTTP_HTTP_REQUEST_H

#include <string>
#include <memory>
#include <map>

#include "tinyrpc/net/abstract_data.h"
#include "tinyrpc/net/http/http_define.h"


namespace tinyrpc {

class HttpRequest : public AbstractData {
 public:
  typedef std::shared_ptr<HttpRequest> ptr; 

 public:
  HttpMethod m_request_method;   
  std::string m_request_path;   
  std::string m_request_query;   
  std::string m_request_version;   
  HttpRequestHeader m_requeset_header;
  std::string m_request_body;   

  std::map<std::string, std::string> m_query_maps;


};

}


#endif
