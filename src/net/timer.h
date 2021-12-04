#ifndef TINYRPC_NET_TIMER_H
#define TINYRPC_NET_TIMER_H


#include <memory>
#include <functional>


namespace tinyrpc {


class TimerEvent {
 public:

  typedef td::shared_ptr<TimerEvent> ptr;
  TimerEvent(int64_t interval, bool is_repeated, std::function<void()>task)
    : m_interval(interval), m_is_repeated(is_repeated), m_task(task) {
    
  }


 private:
  int64_t m_arrive_time;   // when to excute task
  int64_t m_interval;     // interval between two tasks
  bool m_is_repeated;
  std::function<void()> m_task;


};

class Timer {

 public:
  Timer();




};



}

#endif
