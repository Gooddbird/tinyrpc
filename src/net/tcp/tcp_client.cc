#include <sys/socket.h>
#include <arpa/inet.h>
#include "../../comm/log.h"
#include "../../coroutine/coroutine.h"
#include "../../coroutine/coroutine_hook.h"
#include "../../coroutine/coroutine_pool.h"
#include "../net_address.h"
#include "tcp_client.h" 

namespace tinyrpc {

TcpClient::TcpClient(NetAddress::ptr addr) : m_peer_addr(addr) {

  m_family = m_peer_addr->getFamily();
  m_fd = socket(AF_INET, SOCK_STREAM, 0);
  // m_connect_cor = std::make_shared<Coroutine>(128 * 1024, std::bind(&TcpClient::MainConnectCorFunc, this));
  m_connect_cor = GetCoroutinePool()->getCoroutineInstanse();
  m_connect_cor->setCallBack(std::bind(&TcpClient::MainConnectCorFunc, this));

  m_wait_reply_cor = GetCoroutinePool()->getCoroutineInstanse();
  m_wait_reply_cor->setCallBack(std::bind(&TcpClient::WaitReplyCorFunc, this));

  m_reactor = Reactor::GetReactor();
  m_connection = std::make_shared<TcpConnection>(this, m_reactor, m_fd, 128, m_peer_addr);
  assert(m_reactor != nullptr);

}

TcpClient::~TcpClient() {
  GetCoroutinePool()->returnCoroutine(m_connect_cor->getCorId());
  if (m_fd > 0) {
    close(m_fd);
  }
}



TcpConnection* TcpClient::getConnection() {
  if (!m_connection.get()) {
    m_connection = std::make_shared<TcpConnection>(this, m_reactor, m_fd, 128, m_peer_addr);
  }
  return m_connection.get();
}

bool TcpClient::connectAndSend() {
  if (m_connection->getState() == Connected) {
    m_connection->setUpClient();
    m_reactor->addTask(std::bind(&TcpConnection::asyncWrite, m_connection.get()));
    // m_reactor->loop();
  } else {
    // when this coroutine back identify: 
    // case1: connect succ and put task to send data
    // case2: connect error
    Coroutine::Resume(m_connect_cor.get());
  }

  if (m_connection->getState() != Connected) {
    ErrorLog << "connect error, peer addr:[" << m_peer_addr->toString() << "]";
    return false;
  }
  // when this coroutine identify: 
  // case1: succ recv server back package
  // case2: failed recv server back package, it is possible timeout or other resons. I will finish it in the fiture.
  Coroutine::Resume(m_wait_reply_cor.get());

  return true;
  // m_reactor->loop();
}

void TcpClient::onGetReply() {
  Coroutine::Resume(m_wait_reply_cor.get());
}

void TcpClient::start() {
  m_is_stop = false;
  m_reactor->loop();
}

void TcpClient::stop() {
  if (!m_is_stop) {
    m_is_stop = true;
    m_reactor->stop();
  }
}

void TcpClient::MainConnectCorFunc() {
  int n = m_try_counts;
  while (n > 0) {
    int rt = connect_hook(m_fd, reinterpret_cast<sockaddr*>(m_peer_addr->getSockAddr()), m_peer_addr->getSockLen());
    if (rt == 0) {
      m_connection->setUpClient();
      DebugLog << "connect [" << m_peer_addr->toString() << "] succ!";
      m_reactor->addTask(std::bind(&TcpConnection::asyncWrite, m_connection.get()));
      break;
    }
    n--;
  }
}

void TcpClient::WaitReplyCorFunc() {
  while(!m_is_stop) {
    sleep_hook(5);
    break;
  }
}


  
} // namespace name
