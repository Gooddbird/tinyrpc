#ifndef TINYRPC_NET_TCP_IO_THREAD_H
#define TINYRPC_NET_TCP_IO_THREAD_H

#include <memory>
#include <map>
#include "../reactor.h"
#include "../../coroutine/coroutine.h"


namespace tinyrpc {

static thread_local Reactor* t_reactor_ptr = nullptr;

class IOThread {

 public:
  
  typedef std::shared_ptr<IOThread> ptr;
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

  Reactor* getReactor() const {
    return m_reactor;
  }

 private:
 	static void* main(void* arg) {
    assert(t_reactor_ptr == nullptr);
		t_reactor_ptr = new Reactor(); 
		IOThread* thread = static_cast<IOThread*>(arg);
		thread->m_reactor = t_reactor_ptr;

		t_reactor_ptr->getTimer();

    // create main coroutine
    Coroutine::GetCurrentCoroutine();

		t_reactor_ptr->loop();

		return nullptr;
	}

 private:
 	Reactor* m_reactor;

	pthread_t m_thread;
	pid_t m_tid;

};

class IOThreadPool {

 public:
  typedef std::shared_ptr<IOThreadPool> ptr;

  IOThreadPool(int size) : m_size(size) {
    m_io_threads.resize(size);
    for (int i = 0; i < size; ++i) {
      m_io_threads[i] = std::make_shared<IOThread>();
    }  
  }

  IOThread* getIOThread() {
    if (m_index == m_size || m_index == -1) {
      m_index = 0;
    }
    return m_io_threads[m_index++].get();
  }

 private:
  int m_size {0};
  int m_index {-1};

  std::vector<IOThread::ptr> m_io_threads;
  
};


}

#endif
