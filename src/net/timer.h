#ifndef TINYRPC_NET_TIMER_H
#define TINYRPC_NET_TIMER_H

#include <time.h>
#include <memory>
#include <map>
#include <functional>
#include "mutex.h"
#include "reactor.h"


namespace tinyrpc {

int64_t getNowMs() {
  timeval val;
  gettimeofday(&val, nullptr);
  int64_t re = val.tv_sec * 1000 + val.tv_usec / 1000;
  return re;
}

class TimerEvent {

 public:

  typedef std::shared_ptr<TimerEvent> ptr;
  TimerEvent(int64_t interval, bool is_repeated, std::function<void()>task)
    : m_interval(interval), m_is_repeated(is_repeated), m_task(task) {
    m_arrive_time = getNowMs() + m_interval;  	
  }

  void resetTime() {
    m_arrive_time = getNowMs() + m_interval;  	
  }

 public:
  int64_t m_arrive_time;   // when to excute task, ms
  int64_t m_interval;     // interval between two tasks, ms
  bool m_is_repeated {false};
	bool m_is_cancled {false};
  std::function<void()> m_task;

};

class Timer : public FdEvent {

 public:
  
  Timer();

	~Timer();

	int64_t getNextTaskInterval();

	void addTimerEvent(TimerEvent::ptr event, bool need_reset = true);

	void delTimerEvent(TimerEvent::ptr event);

	void resetArriveTime();

  void onTimer();

  void setReactor(Reactor* reactor) {
    m_reactor = reactor;
  }

  Reactor* getReactor() {
    return m_reactor;
  }
	

 private:

 	std::multimap<int64_t, TimerEvent::ptr> m_pending_events;


};



}

#endif
