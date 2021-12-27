#include <memory>
#include <vector>
#include "../../log/log.h"
#include "../fd_event.h"
#include "../reactor.h"
#include "tcp_buffer.h"

namespace tinyrpc {

enum TcpConnectionState {
	Connected = 1,		// can do io
	Closing = 2,			// can do io
	Closed = 3,				// can't do io
};


class TcpConection : public FdEvent {

 public:
 	typedef std::shared_ptr<TcpConection> ptr;

	explicit TcpConection(tinyrpc::Reactor* reactor);


 private:

  void asyncRead(std::vector<char>& re, int size);

	void asyncWrite(const std::vector<char>& buf);

	void onReadEvent();

	void onWriteEvent();

 private:

  TcpConnectionState m_state {TcpConnectionState::Connected};

	TcpBuffer::ptr m_read_buffer;
	TcpBuffer::ptr m_write_buffer;

};


}
