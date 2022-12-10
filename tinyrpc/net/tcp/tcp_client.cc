#include <sys/socket.h>
#include <arpa/inet.h>
#include "tinyrpc/comm/log.h"
#include "tinyrpc/coroutine/coroutine.h"
#include "tinyrpc/coroutine/coroutine_hook.h"
#include "tinyrpc/coroutine/coroutine_pool.h"
#include "tinyrpc/net/net_address.h"
#include "tinyrpc/net/tcp/tcp_client.h" 
#include "tinyrpc/comm/error_code.h"
#include "tinyrpc/net/timer.h"
#include "tinyrpc/net/fd_event.h"
#include "tinyrpc/net/http/http_codec.h"
#include "tinyrpc/net/tinypb/tinypb_codec.h"

namespace tinyrpc {

TcpClient::TcpClient(NetAddress::ptr addr, ProtocalType type /*= TinyPb_Protocal*/) : m_peer_addr(addr) {

  m_family = m_peer_addr->getFamily();
  m_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (m_fd == -1) {
    ErrorLog << "call socket error, fd=-1, sys error=" << strerror(errno);
  }
  DebugLog << "TcpClient() create fd = " << m_fd;
  m_local_addr = std::make_shared<tinyrpc::IPAddress>("127.0.0.1", 0);
  m_reactor = Reactor::GetReactor();

  if (type == Http_Protocal) {
		m_codec = std::make_shared<HttpCodeC>();
	} else {
		m_codec = std::make_shared<TinyPbCodeC>();
	}

  m_connection = std::make_shared<TcpConnection>(this, m_reactor, m_fd, 128, m_peer_addr);

}

TcpClient::~TcpClient() {
  if (m_fd > 0) {
    FdEventContainer::GetFdContainer()->getFdEvent(m_fd)->unregisterFromReactor(); 
    close(m_fd);
    DebugLog << "~TcpClient() close fd = " << m_fd;
  }
}

TcpConnection* TcpClient::getConnection() {
  if (!m_connection.get()) {
    m_connection = std::make_shared<TcpConnection>(this, m_reactor, m_fd, 128, m_peer_addr);
  }
  return m_connection.get();
}
void TcpClient::resetFd() {
  tinyrpc::FdEvent::ptr fd_event = tinyrpc::FdEventContainer::GetFdContainer()->getFdEvent(m_fd);
  fd_event->unregisterFromReactor();
  close(m_fd);
  m_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (m_fd == -1) {
    ErrorLog << "call socket error, fd=-1, sys error=" << strerror(errno);
  } else {

  }
}

int TcpClient::sendAndRecvTinyPb(const std::string& msg_no, TinyPbStruct::pb_ptr& res) {
  bool is_timeout = false;
  tinyrpc::Coroutine* cur_cor = tinyrpc::Coroutine::GetCurrentCoroutine();
  auto timer_cb = [this, &is_timeout, cur_cor]() {
    InfoLog << "TcpClient timer out event occur";
    is_timeout = true;
    this->m_connection->setOverTimeFlag(true); 
    tinyrpc::Coroutine::Resume(cur_cor);
  };
  TimerEvent::ptr event = std::make_shared<TimerEvent>(m_max_timeout, false, timer_cb);
  m_reactor->getTimer()->addTimerEvent(event);

  DebugLog << "add rpc timer event, timeout on " << event->m_arrive_time;

  while (!is_timeout) {
    DebugLog << "begin to connect";
    if (m_connection->getState() != Connected) {
      int rt = connect_hook(m_fd, reinterpret_cast<sockaddr*>(m_peer_addr->getSockAddr()), m_peer_addr->getSockLen());
      if (rt == 0) {
        DebugLog << "connect [" << m_peer_addr->toString() << "] succ!";
        m_connection->setUpClient();
        break;
      }
      resetFd();
      if (is_timeout) {
        InfoLog << "connect timeout, break";
        goto err_deal;
      }
      if (errno == ECONNREFUSED) {
        std::stringstream ss;
        ss << "connect error, peer[ " << m_peer_addr->toString() <<  " ] closed.";
        m_err_info = ss.str();
        ErrorLog << "cancle overtime event, err info=" << m_err_info;
        m_reactor->getTimer()->delTimerEvent(event);
        return ERROR_PEER_CLOSED;
      }
      if (errno == EAFNOSUPPORT) {
        std::stringstream ss;
        ss << "connect cur sys ror, errinfo is " << std::string(strerror(errno)) <<  " ] closed.";
        m_err_info = ss.str();
        ErrorLog << "cancle overtime event, err info=" << m_err_info;
        m_reactor->getTimer()->delTimerEvent(event);
        return ERROR_CONNECT_SYS_ERR;

      } 
    } else {
      break;
    }
  }    

  if (m_connection->getState() != Connected) {
    std::stringstream ss;
    ss << "connect peer addr[" << m_peer_addr->toString() << "] error. sys error=" << strerror(errno);
    m_err_info = ss.str();
    m_reactor->getTimer()->delTimerEvent(event);
    return ERROR_FAILED_CONNECT;
  }

  m_connection->setUpClient();
  m_connection->output();
  if (m_connection->getOverTimerFlag()) {
    InfoLog << "send data over time";
    is_timeout = true;
    goto err_deal;
  }

  while (!m_connection->getResPackageData(msg_no, res)) {
    DebugLog << "redo getResPackageData";
    m_connection->input();

    if (m_connection->getOverTimerFlag()) {
      InfoLog << "read data over time";
      is_timeout = true;
      goto err_deal;
    }
    if (m_connection->getState() == Closed) {
      InfoLog << "peer close";
      goto err_deal;
    }

    m_connection->execute();

  }

  m_reactor->getTimer()->delTimerEvent(event);
  m_err_info = "";
  return 0;

err_deal:
  // connect error should close fd and reopen new one
  FdEventContainer::GetFdContainer()->getFdEvent(m_fd)->unregisterFromReactor();
  close(m_fd);
  m_fd = socket(AF_INET, SOCK_STREAM, 0);
  std::stringstream ss;
  if (is_timeout) {
    ss << "call rpc falied, over " << m_max_timeout << " ms";
    m_err_info = ss.str();

    m_connection->setOverTimeFlag(false);
    return ERROR_RPC_CALL_TIMEOUT;
  } else {
    ss << "call rpc falied, peer closed [" << m_peer_addr->toString() << "]";
    m_err_info = ss.str();
    return ERROR_PEER_CLOSED;
  }

}

void TcpClient::stop() {
  if (!m_is_stop) {
    m_is_stop = true;
    m_reactor->stop();
  }
}

} // namespace name
