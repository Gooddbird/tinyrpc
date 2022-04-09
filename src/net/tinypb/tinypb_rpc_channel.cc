#include <memory>
#include <google/protobuf/service.h>
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include "../net_address.h"
#include "../tcp/tcp_client.h"
#include "tinypb_rpc_channel.h"
#include "tinypb_codec.h"
#include "tinypb_data.h"
#include "../../comm/log.h"


namespace tinyrpc {

TinyPbRpcChannel::TinyPbRpcChannel(NetAddress::ptr addr) {
  m_client = std::make_shared<TcpClient>(addr);
}

void TinyPbRpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method, 
    google::protobuf::RpcController* controller, 
    const google::protobuf::Message* request, 
    google::protobuf::Message* response, 
    google::protobuf::Closure* done) {

  TinyPbStruct pb_struct;
  pb_struct.service_full_name = method->full_name();
  DebugLog << "call service_name = " << pb_struct.service_full_name;
  if (!request->SerializeToString(&(pb_struct.pb_data))) {
    ErrorLog << "serialize send package error";
    return;
  }
  TinyPbCodeC* m_codec = m_client->getConnection()->getCodec();
  m_codec->encode(m_client->getConnection()->getOutBuffer(), &pb_struct);
  InfoLog<< "============================================================";
  InfoLog<< "Set client send request data:\n" << request->DebugString();
  InfoLog<< "============================================================";

  m_client->sendAndRecv();

  TinyPbStruct res_data;
  if (!m_client->getConnection()->getResPackageData(res_data)) {
    ErrorLog << "get reply package empty";
    return;
  }
  if (!response->ParseFromString(res_data.pb_data)) {
    ErrorLog << "parse return package error";
  }
  InfoLog<< "============================================================";
  InfoLog<< "Get server reply response data:\n" << response->DebugString();
  InfoLog<< "============================================================";
  done->Run();
}


}