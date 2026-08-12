#include "ftp_server.h"
#include "tinyrpc/comm/log.h"
#include "tinyrpc/net/tcp/tcp_server.h"

namespace tinyrpc {

FtpServer::FtpServer(NetAddress::ptr addr, const std::string& root_dir)
    :TcpServer(addr,Ftp_Protocal), m_root_dir(root_dir) {
  // m_tcp_server = std::make_shared<TcpServer>(m_addr, Ftp_Protocal);

  // // Override root directory on the dispatcher
  auto* ftp_disp = dynamic_cast<FtpDispatcher*>(getDispatcher().get());
  if (ftp_disp) {
    ftp_disp->setRootDir(m_root_dir);
    InfoLog << "FtpDispatcher root dir set to " << m_root_dir;
  }

  InfoLog << "FtpServer created on " << addr->toString() << ", root=" << m_root_dir;
}

FtpServer::~FtpServer() {}

// void FtpServer::start() {
//   m_tcp_server->start();
// }

}  // namespace tinyrpc
