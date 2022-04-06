#ifndef TINYRPC_NET_TCP_TCP_CONNECTION_H
#define TINYRPC_NET_TCP_TCP_CONNECTION_H

#include <memory>
#include <vector>
#include <queue>
#include "../../comm/log.h"
#include "../fd_event.h"
#include "../reactor.h"
#include "tcp_buffer.h"
#include "../../coroutine/coroutine.h"
#include "../http/http_request.h"
#include "../tinypb/tinypb_codec.h"
#include "io_thread.h"
#include "tcp_connection_time_wheel.h"
#include "abstract_slot.h"
#include "../net_address.h"

namespace tinyrpc {

class TcpServer;
class TcpClient;
class IOThread;

enum TcpConnectionState {
	NotConnected = 1,		// can do io
	Connected = 2,		// can do io
	HalfClosing = 3,			// server call shutdown, write half close
	Closed = 4,				// can't do io
};


class TcpConnection : public std::enable_shared_from_this<TcpConnection> {

 public:
 	typedef std::shared_ptr<TcpConnection> ptr;

	TcpConnection(tinyrpc::TcpServer* tcp_svr, tinyrpc::IOThread* io_thread, int fd, int buff_size, NetAddress::ptr peer_addr);

	TcpConnection(tinyrpc::TcpClient* tcp_cli, tinyrpc::Reactor* reactor, int fd, int buff_size, NetAddress::ptr peer_addr);

  void setUpClient();

	~TcpConnection();

  void initBuffer(int size);

  enum ConnectionType {
    ServerConnection = 1,     // own by server
    ClientConnection = 2,     // own by client
  };

 public:

  void asyncRead();

	void asyncWrite();

  void shutdownConnection();

  TcpConnectionState getState() const;

  TcpBuffer* getInBuffer();

  TcpBuffer* getOutBuffer();

  TinyPbCodeC* getCodec() const;

  const TinyPbStruct* getResPackageData();

  void registerToTimeWheel();

  // void resumeReadCoroutine();

  // void resumeWriteCoroutine();

 public:
  void MainReadCoFunc();

  void MainWriteCoFunc();

 private:
  void clearClient();
  
  void execute();


 private:
  TcpServer* m_tcp_svr {nullptr};
  TcpClient* m_tcp_cli {nullptr};
  IOThread* m_io_thread {nullptr};
  Reactor* m_reactor {nullptr};

  int m_fd {-1};
  TcpConnectionState m_state {TcpConnectionState::Connected};
  ConnectionType m_connection_type {ServerConnection};

  NetAddress::ptr m_peer_addr;


	TcpBuffer::ptr m_read_buffer;
	TcpBuffer::ptr m_write_buffer;
  Coroutine::ptr m_read_cor;
  Coroutine::ptr m_write_cor;

  TinyPbCodeC::ptr m_codec;

  FdEvent::ptr m_fd_event;
  bool m_stop_read {false};
  bool m_stop_write {false};
  std::queue<TinyPbStruct*> m_client_res_data_queue;

  // AbstractSlot<TcpConnection>* m_conn_slot {nullptr};
  std::weak_ptr<AbstractSlot<TcpConnection>> m_weak_slot;

};

}

#endif
