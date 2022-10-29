#include <sys/socket.h>
#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include "tinyrpc/net/tcp/tcp_server.h"
#include "tinyrpc/net/tcp/tcp_connection.h"
#include "tinyrpc/net/tcp/io_thread.h"
#include "tinyrpc/net/tcp/tcp_connection_time_wheel.h"
#include "tinyrpc/coroutine/coroutine.h"
#include "tinyrpc/coroutine/coroutine_hook.h"
#include "tinyrpc/coroutine/coroutine_pool.h"
#include "tinyrpc/comm/config.h"
#include "tinyrpc/net/http/http_codec.h"
#include "tinyrpc/net/http/http_dispatcher.h"
#include "tinyrpc/net/tinypb/tinypb_rpc_dispatcher.h"


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
	int rt = 0;

	if (m_family == AF_INET) {
		sockaddr_in cli_addr;
		memset(&cli_addr, 0, sizeof(cli_addr));
		len = sizeof(cli_addr);
		// call hook accept
		rt = accept_hook(m_fd, reinterpret_cast<sockaddr *>(&cli_addr), &len);
		if (rt == -1) {
			DebugLog << "error, no new client coming, errno=" << errno << "error=" << strerror(errno);
			return -1;
		}
		InfoLog << "New client accepted succ! port:[" << cli_addr.sin_port;
		m_peer_addr = std::make_shared<IPAddress>(cli_addr);
	} else if (m_family == AF_UNIX) {
		sockaddr_un cli_addr;
		len = sizeof(cli_addr);
		memset(&cli_addr, 0, sizeof(cli_addr));
		// call hook accept
		rt = accept_hook(m_fd, reinterpret_cast<sockaddr *>(&cli_addr), &len);
		if (rt == -1) {
			DebugLog << "error, no new client coming, errno=" << errno << "error=" << strerror(errno);
			return -1;
		}
		m_peer_addr = std::make_shared<UnixDomainAddress>(cli_addr);

	} else {
		ErrorLog << "unknown type protocol!";
		close(rt);
		return -1;
	}

	InfoLog << "New client accepted succ! fd:[" << rt <<  ", addr:[" << m_peer_addr->toString() << "]";
	return rt;	
}


TcpServer::TcpServer(NetAddress::ptr addr, ProtocalType type /*= TinyPb_Protocal*/) : m_addr(addr) {

  m_io_pool = std::make_shared<IOThreadPool>(gRpcConfig->m_iothread_num);
	if (type == Http_Protocal) {
		m_dispatcher = std::make_shared<HttpDispacther>();
		m_codec = std::make_shared<HttpCodeC>();
		m_protocal_type = Http_Protocal;
	} else {
		m_dispatcher = std::make_shared<TinyPbRpcDispacther>();
		m_codec = std::make_shared<TinyPbCodeC>();
		m_protocal_type = TinyPb_Protocal;
	}

	m_main_reactor = tinyrpc::Reactor::GetReactor();
	m_main_reactor->setReactorType(MainReactor);

	m_time_wheel = std::make_shared<TcpTimeWheel>(m_main_reactor, gRpcConfig->m_timewheel_bucket_num, gRpcConfig->m_timewheel_inteval);

	m_clear_clent_timer_event = std::make_shared<TimerEvent>(10000, true, std::bind(&TcpServer::ClearClientTimerFunc, this));
	m_main_reactor->getTimer()->addTimerEvent(m_clear_clent_timer_event);

	InfoLog << "TcpServer setup on [" << m_addr->toString() << "]";
}

void TcpServer::start() {

	m_acceptor.reset(new TcpAcceptor(m_addr));
  m_acceptor->init();
	m_accept_cor = GetCoroutinePool()->getCoroutineInstanse();
	m_accept_cor->setCallBack(std::bind(&TcpServer::MainAcceptCorFunc, this));

	InfoLog << "resume accept coroutine";
	tinyrpc::Coroutine::Resume(m_accept_cor.get());

  m_io_pool->start();
	m_main_reactor->loop();

}

TcpServer::~TcpServer() {
	GetCoroutinePool()->returnCoroutine(m_accept_cor);
  DebugLog << "~TcpServer";
}


void TcpServer::MainAcceptCorFunc() {

  while (!m_is_stop_accept) {

    int fd = m_acceptor->toAccept();
    if (fd == -1) {
      ErrorLog << "accept ret -1 error, return, to yield";
      Coroutine::Yield();
      continue;
    }
    IOThread *io_thread = m_io_pool->getIOThread();
		TcpConnection::ptr conn = addClient(io_thread, fd);
		conn->initServer();
		DebugLog << "tcpconnection address is " << conn.get() << ", and fd is" << fd;

    // auto cb = [io_thread, conn]() mutable {
    //   io_thread->addClient(conn.get());
		// 	conn.reset();
    // };

    io_thread->getReactor()->addCoroutine(conn->getCoroutine());
    m_tcp_counts++;
    DebugLog << "current tcp connection count is [" << m_tcp_counts << "]";
  }
}


void TcpServer::addCoroutine(Coroutine::ptr cor) {
	m_main_reactor->addCoroutine(cor);
}

bool TcpServer::registerService(std::shared_ptr<google::protobuf::Service> service) {
	if (m_protocal_type == TinyPb_Protocal) {
		if (service) {
			dynamic_cast<TinyPbRpcDispacther*>(m_dispatcher.get())->registerService(service);
		} else {
			ErrorLog << "register service error, service ptr is nullptr";
			return false;
		}
	} else {
		ErrorLog << "register service error. Just TinyPB protocal server need to resgister Service";
		return false;
	}
	return true;
}

bool TcpServer::registerHttpServlet(const std::string& url_path, HttpServlet::ptr servlet) {
	if (m_protocal_type == Http_Protocal) {
		if (servlet) {
			dynamic_cast<HttpDispacther*>(m_dispatcher.get())->registerServlet(url_path, servlet);
		} else {
			ErrorLog << "register http servlet error, servlet ptr is nullptr";
			return false;
		}
	} else {
		ErrorLog << "register http servlet error. Just Http protocal server need to resgister HttpServlet";
		return false;
	}
	return true;
}


TcpConnection::ptr TcpServer::addClient(IOThread* io_thread, int fd) {
  auto it = m_clients.find(fd);
  if (it != m_clients.end()) {
		it->second.reset();
    // set new Tcpconnection	
		DebugLog << "fd " << fd << "have exist, reset it";
		it->second = std::make_shared<TcpConnection>(this, io_thread, fd, 128, getPeerAddr());
		return it->second;

  } else {
		DebugLog << "fd " << fd << "did't exist, new it";
    TcpConnection::ptr conn = std::make_shared<TcpConnection>(this, io_thread, fd, 128, getPeerAddr()); 
    m_clients.insert(std::make_pair(fd, conn));
		return conn;
  }
}



void TcpServer::freshTcpConnection(TcpTimeWheel::TcpConnectionSlot::ptr slot) {
	auto cb = [slot, this]() mutable {
		this->getTimeWheel()->fresh(slot);	
		slot.reset();
	};
	m_main_reactor->addTask(cb);
}


void TcpServer::ClearClientTimerFunc() {
  // DebugLog << "this IOThread loop timer excute";
  
  // delete Closed TcpConnection per loop
  // for free memory
	// DebugLog << "m_clients.size=" << m_clients.size();
  for (auto &i : m_clients) {
    // TcpConnection::ptr s_conn = i.second;
		// DebugLog << "state = " << s_conn->getState();
    if (i.second && i.second.use_count() > 0 && i.second->getState() == Closed) {
      // need to delete TcpConnection
      DebugLog << "TcpConection [fd:" << i.first << "] will delete, state=" << i.second->getState();
      (i.second).reset();
      // s_conn.reset();
    }
	
  }
}

NetAddress::ptr TcpServer::getPeerAddr() {
	return m_acceptor->getPeerAddr();
}

NetAddress::ptr TcpServer::getLocalAddr() {
	return m_addr;
}

TcpTimeWheel::ptr TcpServer::getTimeWheel() {
  return m_time_wheel;
}

IOThreadPool::ptr TcpServer::getIOThreadPool() {
	return m_io_pool;
}


AbstractDispatcher::ptr TcpServer::getDispatcher() {	
	return m_dispatcher;	
}

AbstractCodeC::ptr TcpServer::getCodec() {
	return m_codec;
}

}
