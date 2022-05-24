#include <google/protobuf/service.h>
#include "tinyrpc/comm/start.h"
#include "tinyrpc/comm/log.h"
#include "tinyrpc/comm/config.h"
#include "tinyrpc/net/tcp/tcp_server.h"

namespace tinyrpc {

tinyrpc::Config::ptr gRpcConfig;
tinyrpc::Logger::ptr gRpcLogger;
tinyrpc::TcpServer::ptr gRpcServer;

static int g_init_config = 0;

void InitConfig(const char* file) {
  if (g_init_config == 0) {
    gRpcConfig = std::make_shared<tinyrpc::Config>(file);
    gRpcConfig->readConf();
    g_init_config = 1;
  }
}

void RegisterService(google::protobuf::Service* service) {
  gRpcServer->registerService(service);
}

TcpServer::ptr GetRpcServer() {
  return gRpcServer;
}

void StartRpcServer() {
  gRpcServer->start();
}


}