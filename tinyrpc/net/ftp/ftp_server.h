#ifndef TINYRPC_NET_FTP_FTP_SERVER_H
#define TINYRPC_NET_FTP_FTP_SERVER_H

#include <memory>
#include <string>
#include "tinyrpc/net/tcp/tcp_server.h"
#include "tinyrpc/net/net_address.h"
#include "tinyrpc/net/ftp/ftp_codec.h"
#include "tinyrpc/net/ftp/ftp_dispatcher.h"

namespace tinyrpc {

class FtpServer:public TcpServer {
 public:
  typedef std::shared_ptr<FtpServer> ptr;

  FtpServer(NetAddress::ptr addr, const std::string& root_dir = "/tmp/ftp");
  ~FtpServer();

  // void start();

 private:
  // NetAddress::ptr m_addr;
  std::string m_root_dir;
  // TcpServer::ptr m_tcp_server;
};

}  // namespace tinyrpc

#endif  // TINYRPC_NET_FTP_FTP_SERVER_H
