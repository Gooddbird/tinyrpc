#include "ftp_dispatcher.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include "tinyrpc/coroutine/coroutine.h"
#include "tinyrpc/coroutine/coroutine_hook.h"
#include "tinyrpc/comm/log.h"

namespace tinyrpc {

FtpDispatcher::FtpDispatcher(const std::string& root_dir) : m_root_dir(root_dir) {}

FtpDispatcher::~FtpDispatcher() {}

void FtpDispatcher::dispatch(AbstractData* data, TcpConnection* conn) {
  FtpCommand* cmd = dynamic_cast<FtpCommand*>(data);
  if (!cmd) {
    ErrorLog << "ftp dispatch: data is not FtpCommand";
    return;
  }

  Coroutine* cur_cor = Coroutine::GetCurrentCoroutine();
  FtpHandler::ptr handler;
  auto it = m_handlers.find(cur_cor);
  if (it == m_handlers.end()) {
    handler = std::make_shared<FtpHandler>(m_root_dir);
    m_handlers[cur_cor] = handler;
  } else {
    handler = it->second;
  }

  FtpResponse::ptr resp;

  // Commands that need a data connection
  if (cmd->m_type == LIST || cmd->m_type == NLST ||
      cmd->m_type == RETR || cmd->m_type == STOR || cmd->m_type == APPE) {

    int data_fd = openDataConnection(handler.get());
    if (data_fd < 0) {
      resp = FtpUtils::MakeResponse(425, "Cannot open data connection");
    } else {
      auto opening = FtpUtils::MakeResponse(150, "Opening data connection");
      conn->getCodec()->encode(conn->getOutBuffer(), opening.get());

      // Flush 150 response to control connection socket before starting data transfer.
      // FTP protocol requires 150 to arrive before data, but the normal output()
      // phase runs after execute() returns, which is too late.
      {
        TcpBuffer* out_buf = conn->getOutBuffer();
        if (out_buf->readAble() > 0) {
          int ctrl_fd = conn->getFd();
          int total_size = out_buf->readAble();
          int read_index = out_buf->readIndex();
          write_hook(ctrl_fd, &(out_buf->m_buffer[read_index]), total_size);
          out_buf->recycleRead(total_size);
          DebugLog << "ftp flushed 150 response to control connection";
        }
      }

      if (cmd->m_type == LIST || cmd->m_type == NLST) {
        resp = handler->handleLIST(cmd->m_arg, data_fd);
      } else if (cmd->m_type == RETR) {
        resp = handler->handleRETR(cmd->m_arg, data_fd);
      } else if (cmd->m_type == STOR || cmd->m_type == APPE) {
        resp = handler->handleSTOR(cmd->m_arg, data_fd);
      }

      close(data_fd);

      auto complete = FtpUtils::MakeResponse(226, "Transfer complete");
      conn->getCodec()->encode(conn->getOutBuffer(), complete.get());
      // Flush 226 response too, same reason as 150 above
      {
        TcpBuffer* out_buf = conn->getOutBuffer();
        if (out_buf->readAble() > 0) {
          int ctrl_fd = conn->getFd();
          int total_size = out_buf->readAble();
          int read_index = out_buf->readIndex();
          write_hook(ctrl_fd, &(out_buf->m_buffer[read_index]), total_size);
          out_buf->recycleRead(total_size);
          DebugLog << "ftp flushed 226 response to control connection";
        }
      }
    }
  } else {
    resp = handler->handleCommand(cmd);
  }

  if (resp) {
    DebugLog << "ftp dispatch: encoding response code=" << resp->m_code;
    conn->getCodec()->encode(conn->getOutBuffer(), resp.get());
    DebugLog << "ftp dispatch: encode done, response sent";
  }

  if (cmd->m_type == QUIT) {
    m_handlers.erase(cur_cor);
    conn->shutdownConnection();
  }
}

int FtpDispatcher::openDataConnection(FtpHandler* handler) {
  if (handler->m_data_mode == FtpHandler::MODE_PASSIVE && handler->m_passive_listen_fd >= 0) {
    int data_fd = accept_hook(handler->m_passive_listen_fd, nullptr, nullptr);
    handler->closePassiveDataConnection();
    handler->m_data_mode = FtpHandler::MODE_PASSIVE;
    return data_fd;
  }

  if (handler->m_data_mode == FtpHandler::MODE_ACTIVE && !handler->m_port_ip.empty()) {
    int data_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (data_fd < 0) return -1;

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(handler->m_port_port);
    inet_pton(AF_INET, handler->m_port_ip.c_str(), &addr.sin_addr);

    int ret = connect_hook(data_fd, (struct sockaddr*)&addr, sizeof(addr));
    if (ret < 0) {
      close(data_fd);
      handler->m_data_mode = FtpHandler::MODE_NONE;
      return -1;
    }

    handler->m_data_mode = FtpHandler::MODE_ACTIVE;
    return data_fd;
  }

  return -1;
}

}  // namespace tinyrpc
