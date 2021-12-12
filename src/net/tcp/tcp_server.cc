#include <sys/socket.h>
#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include "tcp_server.h"


namespace tinyrpc {


TcpAcceptor::TcpAcceptor(Reactor* reactor, NetAddress::ptr net_addr)
	: FdEvent(reactor), m_family(family) {

	m_fd = socket(m_net_addr->getFamily(), SOCK_STREAM, 0);
	assert(m_fd != -1);
	DebugLog << "create listenfd succ, listenfd=" << m_fd;
	int flag = fcntl(m_fd, F_GETFL, 0);
	int rt = fcntl(m_fd, F_SETFL, flag | O_NONBLOCK);
	
	if (rt != 0) {
		ErrorLog << "fcntl set nonblock error, errno=" << errno, "error=" << strerror(errno);
	}
	socklen_t len = sizeof(net_addr->getSockAddr());

	rt = bind(m_fd, net_addr->getSockAddr(), len);
	if (rt != 0) {
		ErrorLog << "bind error, errno=" << errno, "error=" << strerror(errno);
	}

}


TcpAcceptor::~TcpAcceptor() {
	close(m_fd);
}


TcpServer::TcpServer()




void TcpServer::onAccept() {

}

}



