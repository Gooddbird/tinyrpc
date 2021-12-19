#include <memory>
#include "../../log/log.h"
#include "../fd_event.h"
#include "../reactor.h"
#include "tcp_buffer.h"

namespace tinyrpc {

enum TcpConnectionState {
	Connecting = 1,		// can't do io
	Connected = 2,		// can do io
	Closing = 3,			// can do io
	Closed = 4,				// can't do io
};


class TcpConection : public FdEvent {

 public;
 	typedef std::shared_ptr<TcpConection> ptr;

	explicit TcpConection(tinyrpc::Reactor* reactor);


 private:

  void asyncRead();

	void asyncWrite();

	void onReadEvent();

	void onWriteEvent();

 private:

  TcpConnectionState m_state {Connecting};

	TcpBuffer::ptr m_read_buffer;
	TcpBuffer::ptr m_write_buffer;

};


}
