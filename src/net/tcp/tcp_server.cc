#include <sys/socket.h>
#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include "tcp_server.h"


namespace tinyrpc {


TcpAcceptor::TcpAcceptor(Reactor* reactor, NetAddress::ptr net_addr)
	: FdEvent(reactor), m_net_addr(net_addr) {

}

void TcpAcceptor::init() {
	m_fd = socket(m_net_addr->getFamily(), SOCK_STREAM, 0);
	assert(m_fd != -1);
	DebugLog << "create listenfd succ, listenfd=" << m_fd;
	int flag = fcntl(m_fd, F_GETFL, 0);
	int rt = fcntl(m_fd, F_SETFL, flag | O_NONBLOCK);
	
	if (rt != 0) {
		ErrorLog << "fcntl set nonblock error, errno=" << errno << ", error=" << strerror(errno);
	}
	socklen_t len = sizeof(m_net_addr->getSockAddr());

	rt = bind(m_fd, m_net_addr->getSockAddr(), len);
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

}

void TcpServer::onReadCallBack() {

}

void TcpServer::onWriteCallBack() {

}

}



