#ifndef TINYRPC_COMM_START_H
#define TINYRPC_COMM_START_H

#include <google/protobuf/service.h>
#include "../net/tcp/tcp_server.h"

namespace tinyrpc {

void InitConfig(const char* file);

void RegisterService(google::protobuf::Service* service);

void StartRpcServer();

TcpServer::ptr GetRpcServer();

}

#endif