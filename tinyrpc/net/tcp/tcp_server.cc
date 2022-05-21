#include <sys/socket.h>
#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include "tcp_server.h"
#include "tcp_connection.h"
#include "io_thread.h"
#include "tcp_connection_time_wheel.h"
#include "../../coroutine/coroutine.h"
#include "../../coroutine/coroutine_hook.h"
#include "../../coroutine/coroutine_pool.h"
#include "../../comm/config.h"


namespace tinyrpc {

extern tinyrpc::Config::ptr gRpcConfig;

TcpAcceptor::TcpAcceptor(NetAddress::ptr net_addr) : m_local_addr(net_addr) {
	
	m_family = m_local_addr->getFamily();
}

void TcpAcceptor::init() {
	m_fd = socket(m_local_addr->getFamily(), SOCK_STREAM, 0);
	if (m_fd < 0) {
		ErrorLog << "start server error. socket error, sys error=" << strerror(errno);
		Exit(0);
	}
	// assert(m_fd != -1);
	DebugLog << "create listenfd succ, listenfd=" << m_fd;

	// int flag = fcntl(m_fd, F_GETFL, 0);
	// int rt = fcntl(m_fd, F_SETFL, flag | O_NONBLOCK);
	
	// if (rt != 0) {
		// ErrorLog << "fcntl set nonblock error, errno=" << errno << ", error=" << strerror(errno);
	// }

	int val = 1;
	if (setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0) {
		ErrorLog << "set REUSEADDR error";
	}

	socklen_t len = m_local_addr->getSockLen();
	int rt = bind(m_fd, m_local_addr->getSockAddr(), len);
	if (rt != 0) {
		ErrorLog << "start server error. bind error, errno=" << errno << ", error=" << strerror(errno);
		Exit(0);
	}
  // assert(rt == 0);

	DebugLog << "set REUSEADDR succ";
	rt = listen(m_fd, 10);
	if (rt != 0) {
		ErrorLog << "start server error. listen error, fd= " << m_fd << ", errno=" << errno << ", error=" << strerror(errno);
		Exit(0);
	}
  // assert(rt == 0);

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
	int rt = accept_hook(m_fd, reinterpret_cast<sockaddr*>(&cli_addr), &len);
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

	InfoLog << "New client accepted succ! fd:[" << rt <<  ", addr:[" << m_peer_addr->toString() << "]";
	return rt;	
}


TcpServer::TcpServer(NetAddress::ptr addr) : m_addr(addr) {
  m_io_pool = std::make_shared<IOThreadPool>(gRpcConfig->m_iothread_num);
	m_dispatcher = std::make_shared<TinyPbRpcDispacther>();
	m_main_reactor = tinyrpc::Reactor::GetReactor();
	InfoLog << "TcpServer setup on [" << m_addr->toString() << "]";
}

void TcpServer::start() {

	m_acceptor.reset(new TcpAcceptor(m_addr));
	// m_accept_cor = std::make_shared<tinyrpc::Coroutine>(128 * 1024, std::bind(&TcpServer::MainAcceptCorFunc, this)); 

	m_accept_cor = GetCoroutinePool()->getCoroutineInstanse();
	m_accept_cor->setCallBack(std::bind(&TcpServer::MainAcceptCorFunc, this));

	tinyrpc::Coroutine::Resume(m_accept_cor.get());
	m_main_reactor->loop();

}

TcpServer::~TcpServer() {
	GetCoroutinePool()->returnCoroutine(m_accept_cor->getCorId());
  DebugLog << "~TcpServer";
}

NetAddress::ptr TcpServer::getPeerAddr() {
	return m_acceptor->getPeerAddr();
}

void TcpServer::MainAcceptCorFunc() {
  DebugLog << "enable Hook here";

	m_acceptor->init();	
	while(!m_is_stop_accept) {

		int fd = m_acceptor->toAccept();
		if (fd == -1) {
			ErrorLog << "accept ret -1 error, return, to yield";
      Coroutine::Yield();
      continue;
		}
		IOThread* io_thread = m_io_pool->getIOThread();
		auto cb = [this, io_thread, fd]() {
			io_thread->addClient(this, fd);
		};
		io_thread->getReactor()->addTask(cb);
		m_tcp_counts++;
		DebugLog << "current tcp connection count is [" << m_tcp_counts << "]";
	}
}


TinyPbRpcDispacther* TcpServer::getDispatcher() {	
	return m_dispatcher.get();	
}

void TcpServer::addCoroutine(Coroutine::ptr cor) {
	m_main_reactor->addCoroutine(cor);
}

void TcpServer::registerService(google::protobuf::Service* service) {
	m_dispatcher->registerService(service);
}

}
