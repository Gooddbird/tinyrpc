#include <google/protobuf/service.h>
#include "start.h"
#include "log.h"
#include "config.h"
#include "../net/tcp/tcp_server.h"

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