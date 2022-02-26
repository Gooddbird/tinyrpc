#ifndef TINYRPC_NET_TCP_TCP_CONNECTION_H
#define TINYRPC_NET_TCP_TCP_CONNECTION_H

#include <memory>
#include <vector>
#include "../../log/log.h"
#include "../fd_event.h"
#include "../reactor.h"
#include "tcp_buffer.h"
#include "../../coroutine/coroutine.h"

namespace tinyrpc {

enum TcpConnectionState {
	Connected = 1,		// can do io
	HalfClosing = 2,			// server call shutdown, write half close
	Closed = 3,				// can't do io
};


class TcpConection {

 public:
 	typedef std::shared_ptr<TcpConection> ptr;

	TcpConection(tinyrpc::Reactor* reactor, int fd, int buff_size);

	~TcpConection();

  void initBuffer(int size);


 public:

  void asyncRead(std::vector<char>& re, int& size);

	void asyncWrite(const std::vector<char>& buf);

  void shutdownConnection();

 private:
  void MainReadCoFunc();

  void MainWriteCoFunc();

  void clearClient();

 private:

  Reactor* m_reactor;
  int m_fd = -1;
  TcpConnectionState m_state {TcpConnectionState::Connected};

	TcpBuffer::ptr m_read_buffer;
	TcpBuffer::ptr m_write_buffer;
  Coroutine::ptr m_read_cor;
  Coroutine::ptr m_write_cor;

  FdEvent::ptr m_fd_event;
  bool m_stop_read = false;
  bool m_stop_write = false;

};

}

#endif
