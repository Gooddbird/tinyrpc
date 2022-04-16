#include <memory>
#include <google/protobuf/service.h>
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include "../net_address.h"
#include "../tcp/tcp_client.h"
#include "tinypb_rpc_channel.h"
#include "tinypb_rpc_controller.h"
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
  TinyPbRpcController* rpc_controller = dynamic_cast<TinyPbRpcController*>(controller); 
  
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

  int rt = m_client->sendAndRecv();
  if (rt != 0) {
    rpc_controller->SetError(rt, m_client->getErrInfo());
    ErrorLog << "call rpc server error, service_full_name=" << pb_struct.service_full_name << ", error_code=" 
        << rt << ", error_info = " << m_client->getErrInfo();
    return;
  }

  TinyPbStruct res_data;
  if (!m_client->getConnection()->getResPackageData(pb_struct.msg_req, res_data)) {
    ErrorLog << "get reply package empty of msg_rep[" << pb_struct.msg_req << "]";
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