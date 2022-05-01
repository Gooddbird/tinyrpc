#include <google/protobuf/message.h>
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>

#include "../abstract_dispatcher.h"
#include "../../comm/error_code.h"
#include "tinypb_data.h"
#include "tinypb_rpc_dispatcher.h"
#include "tinypb_rpc_controller.h"
#include "tinypb_rpc_closure.h"
#include "tinypb_codec.h"

namespace tinyrpc {

class TcpBuffer;

void TinyPbRpcDispacther::dispatch(AbstractData* data, TcpConnection* conn) {
  TinyPbStruct* tmp = dynamic_cast<TinyPbStruct*>(data);

  if (tmp == nullptr) {
    ErrorLog << "dynamic_cast error";
    return;
  }

  assert(tmp != nullptr);
  std::string service_name;
  std::string method_name;

  TinyPbStruct reply_pk;
  reply_pk.service_full_name = tmp->service_full_name;
  reply_pk.msg_req = tmp->msg_req;

  if (!parseServiceFullName(tmp->service_full_name, service_name, method_name)) {
    ErrorLog << "parse service name " << tmp->service_full_name << "error";

    reply_pk.err_code = ERROR_PARSE_SERVICE_NAME;
    std::stringstream ss;
    ss << "cannot parse service_name:[" << tmp->service_full_name << "]"; 
    reply_pk.err_info = ss.str();
    conn->getCodec()->encode(conn->getOutBuffer(), dynamic_cast<AbstractData*>(&reply_pk));
    return;
  }

  auto it = m_service_map.find(service_name);
  if (it == m_service_map.end() || !((*it).second)) {
    reply_pk.err_code = ERROR_SERVICE_NOT_FOUND;
    std::stringstream ss;
    ss << "not found service_name:[" << service_name << "]"; 
    ErrorLog << ss.str();
    reply_pk.err_info = ss.str();

    conn->getCodec()->encode(conn->getOutBuffer(), dynamic_cast<AbstractData*>(&reply_pk));
    return;

  }

  google::protobuf::Service* service = (*it).second;

  const google::protobuf::MethodDescriptor* method = service->GetDescriptor()->FindMethodByName(method_name);
  if (!method) {
    reply_pk.err_code = ERROR_METHOD_NOT_FOUND;
    std::stringstream ss;
    ss << "not found method_name:[" << method_name << "]"; 
    ErrorLog << ss.str();
    reply_pk.err_info = ss.str();
    conn->getCodec()->encode(conn->getOutBuffer(), dynamic_cast<AbstractData*>(&reply_pk));
    return;
  }

  google::protobuf::Message* request = service->GetRequestPrototype(method).New();
  DebugLog << "request.name = " << request->GetDescriptor()->full_name();

  if(!request->ParseFromString(tmp->pb_data)) {
    reply_pk.err_code = ERROR_FAILED_SERIALIZE;
    std::stringstream ss;
    ss << "faild to parse request data, request.name:[" << request->GetDescriptor()->full_name() << "]";
    reply_pk.err_info = ss.str();
    ErrorLog << ss.str();
    delete request;
    conn->getCodec()->encode(conn->getOutBuffer(), dynamic_cast<AbstractData*>(&reply_pk));
    return;
  }

  InfoLog<< "============================================================";
  InfoLog<< "Get client request data:\n" << request->DebugString();
  InfoLog<< "============================================================";

  google::protobuf::Message* response = service->GetResponsePrototype(method).New();

  DebugLog << "response.name = " << response->GetDescriptor()->full_name();

  TinyPbRpcController* rpc_controller = new TinyPbRpcController();

  std::function<void()> reply_package_func = [&reply_pk, response, request]()
  {
    // DebugLog << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<";
    InfoLog << "Call [" << reply_pk.service_full_name << "] succ, now send reply package";

    if (!(response->SerializeToString(&(reply_pk.pb_data)))) {
      reply_pk.pb_data = "";
      ErrorLog << "reply error! encode reply package error";
      reply_pk.err_code = ERROR_FAILED_SERIALIZE;
      reply_pk.err_info = "failed to serilize relpy data";
    } else {
      InfoLog<< "============================================================";
      InfoLog<< "Set server response data:\n" << response->DebugString();
      InfoLog<< "============================================================";
    }

    delete request;
    delete response;

  };

  TinyPbRpcClosure closure(reply_package_func);
  service->CallMethod(method, rpc_controller, request, response, &closure);

  conn->getCodec()->encode(conn->getOutBuffer(), dynamic_cast<AbstractData*>(&reply_pk));


}


bool TinyPbRpcDispacther::parseServiceFullName(const std::string& full_name, std::string& service_name, std::string& method_name) {
  if (full_name.empty()) {
    ErrorLog << "service_full_name empty";
    return false;
  }
  std::size_t i = full_name.find(".");
  if (i == full_name.npos) {
    ErrorLog << "not found [.]";
    return false;
  }

  service_name = full_name.substr(0, i);
  DebugLog << "service_name = " << service_name;
  method_name = full_name.substr(i + 1, full_name.length() - i - 1);
  DebugLog << "method_name = " << method_name;

  return true;

}

void TinyPbRpcDispacther::registerService(google::protobuf::Service *service) {
  std::string service_name = service->GetDescriptor()->full_name();
  m_service_map[service_name] = service;
  InfoLog << "succ register service[" << service_name << "]!"; 
}

}