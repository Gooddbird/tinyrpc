#include <sys/socket.h>
#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include "tcp_server.h"
#include "tcp_connection.h"
#include "../../coroutine/coroutine.h"
#include "../../coroutine/coroutine_hook.h"

namespace tinyrpc {

TcpAcceptor::TcpAcceptor(Reactor* reactor, NetAddress::ptr net_addr)
	: FdEvent(reactor), m_local_addr(net_addr) {
	
	m_family = m_local_addr->getFamily();
}

void TcpAcceptor::init() {
	m_fd = socket(m_local_addr->getFamily(), SOCK_STREAM, 0);
	assert(m_fd != -1);
	DebugLog << "create listenfd succ, listenfd=" << m_fd;
	int flag = fcntl(m_fd, F_GETFL, 0);
	int rt = fcntl(m_fd, F_SETFL, flag | O_NONBLOCK);
	
	if (rt != 0) {
		ErrorLog << "fcntl set nonblock error, errno=" << errno << ", error=" << strerror(errno);
	}
	socklen_t len = m_local_addr->getSockLen();

	rt = bind(m_fd, m_local_addr->getSockAddr(), len);
	if (rt != 0) {
		ErrorLog << "bind error, errno=" << errno << ", error=" << strerror(errno);
	}

	rt = listen(m_fd, 10);
	if (rt != 0) {
		ErrorLog << "listen error, fd= " << m_fd << ", errno=" << errno << ", error=" << strerror(errno);
	}

	// addListenEvents(IOEvent::READ);
	// updateToReactor();

}

TcpAcceptor::~TcpAcceptor() {
	unregisterFromReactor();	

	if (m_fd != -1) {
		close(m_fd);
	}
}

int TcpAcceptor::toAccept() {

	socklen_t len;
	sockaddr cli_addr;
	
	// call hook accept
	int rt = accept(m_fd, &cli_addr, &len);
	if (rt == -1) {
		DebugLog << "error, no new client coming";
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


TcpServer::TcpServer(NetAddress::ptr addr) : m_addr(addr) {

}

void TcpServer::init() {

	// m_main_reactor = tinyrpc::GetReactor();
	// assert(m_main_reactor != nullptr);

	// m_main_thread_id = m_main_reactor->getTid();

	// m_acceptor.reset(new TcpAcceptor(reactor, m_addr));
	// m_acceptor->setCallBack(IOEvent::READ, std::bind(&TcpServer::onReadCallBack, this));
	// m_acceptor->setCallBack(IOEvent::WRITE, std::bind(&TcpServer::onWriteCallBack, this));

	// m_acceptor->init();

	// m_main_reactor->loop();

	m_main_reactor = tinyrpc::Reactor::GetReactor();
	m_acceptor.reset(new TcpAcceptor(m_main_reactor, m_addr));
	tinyrpc::Coroutine::ptr cor = std::make_shared<tinyrpc::Coroutine>(128 * 1024, std::bind(&TcpServer::MainCorFun, this)); 
	tinyrpc::Coroutine::Resume(cor.get());

	m_main_reactor->loop();

}

TcpServer::~TcpServer() {
	m_main_reactor->stop();
}

void TcpServer::onReadCallBack() {
	// int fd = m_acceptor->accept();

	// if (fd == -1) {
		// ErrorLog << "accept error, return";
	// }
	// m_tcp_counts++;
	// DebugLog << "current tcp connection count is [" << m_tcp_counts << "]";

	// Reactor* reactor = m_main_reactor;
	// TcpConection::ptr tcp_conn = std::make_shared<TcpConection> (reactor);
	// tcp_conn->init(fd, 128);
  // m_clients.push_back(tcp_conn);

}

void TcpServer::onWriteCallBack() {
	
}

void TcpServer::MainCorFun() {
	tinyrpc::enableHook();
	m_acceptor->init();	
	int fd;
	TcpConection::ptr tcp_conn;
	while(!m_is_stop_accept) {
		fd = m_acceptor->toAccept();
		if (fd == -1) {
			ErrorLog << "accept error, return";
			continue;
		}

		m_tcp_counts++;
		DebugLog << "current tcp connection count is [" << m_tcp_counts << "]";

		tcp_conn = std::make_shared<TcpConection> (m_main_reactor);
		tcp_conn->init(fd, 128);
		m_clients.push_back(tcp_conn);

	}
}

}
