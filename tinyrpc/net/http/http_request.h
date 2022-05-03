#ifndef TINYRPC_NET_HTTP_HTTP_REQUEST_H
#define TINYRPC_NET_HTTP_HTTP_REQUEST_H

#include <string>
#include <memory>


namespace tinyrpc {

class HttpRequest {

 public:

  typedef std::shared_ptr<HttpRequest> ptr; 
  enum METHOD {
    GET = 1,
    POST = 2, 
  };

  void setRequertMethod(const METHOD v) {
    m_request_method = v; 
  }

  METHOD getRequertMethod() const {
    return m_request_method;
  }

  void setRequestUrl(const std::string& v) {
    m_request_url = v;
  }

  std::string getRequertUrl() const {
    return m_request_url;
  }
  
  void setRequestHeader(const std::string& v) {
    m_request_header = v;
  }

  std::string getRequertHeader() const {
    return m_request_header;
  }
  
  void setRequestVersion(const std::string& v) {
    m_request_version = v;
  }

  std::string getRequertVersion() const {
    return m_request_version;
  }

  void setRequestBody(const std::string& v) {
    m_request_body = v;
  }

  std::string getRequertBody() const {
    return m_request_body;
  }

 private:
  METHOD m_request_method;   
  std::string m_request_url;   
  std::string m_request_version;   
  std::string m_request_header;   
  std::string m_request_body;   


};

}


#endif
