#ifndef TINYRPC_NET_FTP_FTP_DISPATCHER_H
#define TINYRPC_NET_FTP_FTP_DISPATCHER_H

#include <map>
#include <memory>
#include <string>
#include "tinyrpc/net/abstract_dispatcher.h"
#include "tinyrpc/net/tcp/tcp_connection.h"
#include "tinyrpc/net/ftp/ftp_define.h"
#include "tinyrpc/net/ftp/ftp_handler.h"

namespace tinyrpc {

class FtpDispatcher : public AbstractDispatcher {
 public:
  FtpDispatcher(){};
  FtpDispatcher(const std::string& root_dir);
  ~FtpDispatcher();

  void dispatch(AbstractData* data, TcpConnection* conn);

  void setRootDir(const std::string& root_dir) { m_root_dir = root_dir; }

 private:
  int openDataConnection(FtpHandler* handler);

  std::string m_root_dir;
  std::map<Coroutine*, FtpHandler::ptr> m_handlers;
};

}  // namespace tinyrpc

#endif  // TINYRPC_NET_FTP_FTP_DISPATCHER_H
