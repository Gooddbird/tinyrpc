#ifndef TINYRPC_NET_TCP_TCP_SERVER_H
#define TINYRPC_NET_TCP_TCP_SERVER_H

#include "io_thread.h"
#include "../reactor.h"
#include "../fd_event.h"


namespace tinyrpc {

class TcpServer : public FdEvent {

 public:

	explict TcpServer(Reactor* m_reactor, );



 private:


};

}

#endif
