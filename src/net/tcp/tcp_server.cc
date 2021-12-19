#include <sys/socket.h>
#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include "tcp_server.h"


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

	addListenEvents(IOEvent::READ);
	updateToReactor();

}

TcpAcceptor::~TcpAcceptor() {
	unregisterFromReactor();	

	if (m_fd != -1) {
		close(m_fd);
	}
}

int TcpAcceptor::accept() {

	socklen_t len;
	sockaddr cli_addr;
	sockaddr* addr = &cli_addr;

	int rt = ::accept(m_fd, addr, &len);
	if (rt == -1) {
		DebugLog << "error, no new client coming";
		return -1;
	}

	if (m_family == AF_INET) {
		sockaddr_in* ipv4_addr = reinterpret_cast<sockaddr_in*>(addr); 

		m_peer_addr = std::make_shared<IPAddress>(*ipv4_addr);
	} else if (m_family == AF_UNIX) {

		sockaddr_un* unix_addr = reinterpret_cast<sockaddr_un*>(addr);	

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

	m_main_reactor.reset(new Reactor());
	m_timer = m_main_reactor->getTimer();
	m_main_thread_id = m_main_reactor->getTid();

	Reactor* reactor = m_main_reactor.get();

	m_acceptor.reset(new TcpAcceptor(reactor, m_addr));
	m_acceptor->setCallBack(IOEvent::READ, std::bind(&TcpServer::onReadCallBack, this));
	m_acceptor->setCallBack(IOEvent::WRITE, std::bind(&TcpServer::onWriteCallBack, this));

	m_acceptor->init();

	m_main_reactor->loop();

}

TcpServer::~TcpServer() {
	m_main_reactor->stop();

}

void TcpServer::onReadCallBack() {
	int rt = m_acceptor->accept();

	if (rt == -1) {
		ErrorLog << "accept error, return";
	}
	m_tcp_counts++;
	DebugLog << "current tcp connection count is [" << m_tcp_counts << "]";

}

void TcpServer::onWriteCallBack() {

}

}



