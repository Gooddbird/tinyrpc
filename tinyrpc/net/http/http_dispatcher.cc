#include <google/protobuf/service.h>
#include "tinyrpc/net/http/http_dispatcher.h"
#include "tinyrpc/net/http/http_request.h"


namespace tinyrpc {

void HttpDispacther::dispatch(AbstractData* data, TcpConnection* conn) {}

void HttpDispacther::registerService(google::protobuf::Service* service) {}

}