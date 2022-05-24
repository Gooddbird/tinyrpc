#include <queue>
#include <vector>
#include "tinyrpc/net/tcp/abstract_slot.h"
#include "tinyrpc/net/tcp/tcp_connection.h"
#include "tinyrpc/net/tcp/tcp_connection_time_wheel.h"
#include "tinyrpc/net/timer.h"

namespace tinyrpc {

TcpTimeWheel::TcpTimeWheel(Reactor* reactor, int bucket_count, int inteval /*= 10*/) 
  : m_reactor(reactor)
  , m_bucket_count(bucket_count)
  , m_inteval(inteval) {

  for (int i = 0; i < bucket_count; ++i) {
    std::vector<TcpConnectionSlot::ptr> tmp;
    m_wheel.push(tmp);
  }

  m_event = std::make_shared<TimerEvent>(m_inteval * 1000, true, std::bind(&TcpTimeWheel::loopFunc, this));
  m_reactor->getTimer()->addTimerEvent(m_event);
}


TcpTimeWheel::~TcpTimeWheel() {
  m_reactor->getTimer()->delTimerEvent(m_event);
}

void TcpTimeWheel::loopFunc() {
  // DebugLog << "pop src bucket";
  m_wheel.pop();
  std::vector<TcpConnectionSlot::ptr> tmp;
  m_wheel.push(tmp);
  // DebugLog << "push new bucket";
}

void TcpTimeWheel::fresh(TcpConnectionSlot::ptr slot) {
  DebugLog << "fresh connection";
  m_wheel.back().emplace_back(slot);
}


}