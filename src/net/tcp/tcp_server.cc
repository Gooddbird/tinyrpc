#include <sys/socket.h>
#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include "tcp_server.h"
#include "tcp_connection.h"
#include "io_thread.h"
#include "../../coroutine/coroutine.h"
#include "../../coroutine/coroutine_hook.h"

namespace tinyrpc {

TcpAcceptor::TcpAcceptor(NetAddress::ptr net_addr) : m_local_addr(net_addr) {
	
	m_family = m_local_addr->getFamily();
}

void TcpAcceptor::init() {
	m_fd = socket(m_local_addr->getFamily(), SOCK_STREAM, 0);

	assert(m_fd != -1);
	DebugLog << "create listenfd succ, listenfd=" << m_fd;

	// int flag = fcntl(m_fd, F_GETFL, 0);
	// int rt = fcntl(m_fd, F_SETFL, flag | O_NONBLOCK);
	
	// if (rt != 0) {
		// ErrorLog << "fcntl set nonblock error, errno=" << errno << ", error=" << strerror(errno);
	// }

	socklen_t len = m_local_addr->getSockLen();

	int rt = bind(m_fd, m_local_addr->getSockAddr(), len);
	if (rt != 0) {
		ErrorLog << "bind error, errno=" << errno << ", error=" << strerror(errno);
	}
  assert(rt == 0);

	rt = listen(m_fd, 10);
	if (rt != 0) {
		ErrorLog << "listen error, fd= " << m_fd << ", errno=" << errno << ", error=" << strerror(errno);
	}
  assert(rt == 0);

}

TcpAcceptor::~TcpAcceptor() {
  FdEvent::ptr fd_event = FdEventContainer::GetFdContainer()->getFdEvent(m_fd);
  fd_event->unregisterFromReactor();
	if (m_fd != -1) {
		close(m_fd);
	}
}

int TcpAcceptor::toAccept() {

	socklen_t len = 0;
	sockaddr cli_addr;
	
	// call hook accept
	int rt = accept(m_fd, reinterpret_cast<sockaddr*>(&cli_addr), &len);
	if (rt == -1) {
		DebugLog << "error, no new client coming, errno=" << errno << "error=" << strerror(errno);
		return -1;
	}

	if (m_family == AF_INET) {
		sockaddr_in* ipv4_addr = reinterpret_cast<sockaddr_in*>(&cli_addr); 

		m_peer_addr = std::make_shared<IPAddress>(*ipv4_addr);
	} else if (m_family == AF_UNIX) {

		sockaddr_un* unix_addr = reinterpret_cast<sockaddr_un*>(&cli_addr);	

		m_peer_addr = std::make_shared<UnixDomainAddress>(*unix_addr);
	} else {
		ErrorLog << "unknown type protocol!";
		close(rt);
		return -1;
	}

	DebugLog << "new client coming! fd:[" << rt <<  ", addr:[" << m_peer_addr->toString() << "]";
	return rt;	
}


TcpServer::TcpServer(NetAddress::ptr addr, int pool_size /*=10*/) : m_addr(addr) {
  m_io_pool = std::make_shared<IOThreadPool>(pool_size);
	m_dispatcher = std::make_shared<TinyPbRpcDispacther>();
}

void TcpServer::start() {

	m_main_reactor = tinyrpc::Reactor::GetReactor();
	m_acceptor.reset(new TcpAcceptor(m_addr));
	m_accept_cor = std::make_shared<tinyrpc::Coroutine>(128 * 1024, std::bind(&TcpServer::MainAcceptCorFunc, this)); 
	tinyrpc::Coroutine::Resume(m_accept_cor.get());

  m_timer.reset(m_main_reactor->getTimer());

  m_timer_event = std::make_shared<TimerEvent>(10000, true, 
    std::bind(&TcpServer::MainLoopTimerFunc, this));
  
  m_timer->addTimerEvent(m_timer_event);

	m_main_reactor->loop();

}

TcpServer::~TcpServer() {
  DebugLog << "~TcpServer";
}

void TcpServer::MainAcceptCorFunc() {
  DebugLog << "enable Hook here";
	tinyrpc::enableHook();

	m_acceptor->init();	
	while(!m_is_stop_accept) {

		int fd = m_acceptor->toAccept();
		if (fd == -1) {
			ErrorLog << "accept ret -1 error, return, to yield";
      Coroutine::Yield();
      continue;
		}

		m_tcp_counts++;
		DebugLog << "current tcp connection count is [" << m_tcp_counts << "]";
     
    TcpConnection::ptr tcp_conn = std::make_shared<TcpConnection> (this, 
        m_io_pool->getIOThread()->getReactor(), fd, 128);

    addClient(fd, tcp_conn);
    DebugLog << "insert succ, size=" << m_clients.size();
	}
}


void TcpServer::MainLoopTimerFunc() {
  DebugLog << "this TcpServer loop timer excute";
  
  // delete Closed TcpConnection per loop
  // for free memory
  for (auto &i : m_clients) {
    if ((i.second)->getState() == Closed) {
      // need to delete TcpConnection
      DebugLog << "TcpConection [fd:" << i.first << "] will delete";
      (i.second).reset();
    }
  }

  DebugLog << "this TcpServer loop timer end";
  
}

bool TcpServer::addClient(int fd, const TcpConnection::ptr& conn) {
  auto it = m_clients.find(fd);
  if (it != m_clients.end()) {
    TcpConnection::ptr s_conn = it->second;
    if (s_conn->getState() != Closed) {
      ErrorLog << "insert error, this fd of TcpConection exist and state not Closed";
      return false;
    }
    // src Tcpconnection can delete
    s_conn.reset();
    // set new Tcpconnection
    it->second = conn;
    
  } else {
    m_clients.insert(std::make_pair(fd, conn));
  }
  return true;
}

bool TcpServer::delClient(int fd) {
  auto it = m_clients.find(fd);
  if (it == m_clients.end()) {
    ErrorLog << "this fd of TcpConnection not exist";
    return false;
  }
  // only delete this TcpConnection
  // don't delete map'key
  ((*it).second).reset();
  return true;
}

TinyPbRpcDispacther* TcpServer::getDispatcher() {	
	return m_dispatcher.get();	
}

}
