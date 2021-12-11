#ifndef TINYRPC_NET_TCP_IO_THREAD_H
#define TINYRPC_NET_TCP_IO_THREAD_H

#include <memory>
#include "../reactor.h"


namespace tinyrpc {

static thread_local Reactor* t_reactor_ptr = nullptr;

class IOThread {

 public:
 	IOThread() {
		pthread_create(&m_thread, nullptr, &IOThread::main, this);
	}

	~IOThread() {
		m_reactor->stop();
		pthread_join(m_thread, nullptr);

		if (m_reactor != nullptr) {

			delete m_reactor;
			m_reactor = nullptr;
		}
	}

 private:
 	static void* main(void* arg) {
		t_reactor_ptr = new Reactor(); 
		IOThread* thread = static_cast<IOThread*>(arg);
		thread->m_reactor = t_reactor_ptr;

		t_reactor_ptr->getTimer();

		t_reactor_ptr->loop();

		return nullptr;
	}



 private:
 	Reactor* m_reactor;

	pthread_t m_thread;
	pid_t m_tid;

};


}




#endif
