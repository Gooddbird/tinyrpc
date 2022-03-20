#include <sys/socket.h>
#include <arpa/inet.h>
#include "../../log/log.h"
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

  m_reactor = Reactor::GetReactor();
  m_connection = std::make_shared<TcpConnection>(this, m_reactor, m_fd, 128);
  assert(m_reactor != nullptr);

}

TcpClient::~TcpClient() {
  GetCoroutinePool()->returnCoroutine(m_connect_cor->getCorId());
  if (m_fd > 0) {
    close(m_fd);
  }
}

void TcpClient::onReply() {


}

TcpConnection* TcpClient::getConnection() {
  if (!m_connection.get()) {
  m_connection = std::make_shared<TcpConnection>(this, m_reactor, m_fd, 128);
  }
  return m_connection.get();
}

void TcpClient::start() {
  if (m_connection->getState() == Connected) {
    m_connection->setUpClient();
    m_reactor->addTask(std::bind(&TcpConnection::asyncWrite, m_connection.get()));
    m_reactor->loop();
    return;
  }
  Coroutine::Resume(m_connect_cor.get());
  m_reactor->loop();
}

void TcpClient::stop() {
  m_reactor->stop();
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



  
} // namespace name
