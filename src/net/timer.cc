#include <sys/timerfd.h>
#include <assert.h>
#include <time.h>
#include <string.h>
#include <vector>
#include <sys/time.h>
#include <functional>
#include <map>
#include "../log/log.h"
#include "timer.h"
#include "mutex.h"
#include "fd_event.h"


namespace tinyrpc {

Timer::Timer(Reactor* reactor) : m_reactor(reactor) {

  m_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK|TFD_CLOEXEC);
  if (m_fd == -1) {
    DebugLog << "timerfd_create error";  
  } 
	m_read_callback = std::bind(&Timer::onTimer, this);
  addListenEvents(READ);
  updateToReactor();

}

Timer::~Timer() {
  unregisterFromReactor();
	close(m_fd);
}


void Timer::addTimerEvent(TimerEvent::ptr event, bool need_reset /*=true*/) {
  bool is_reset = false;
  auto it = m_pending_events.begin();
  if (it == m_pending_events.end()) {
    is_reset = true;
  } else {
    if (event->m_arrive_time < (*it).second->m_arrive_time) {
      is_reset = true;
    }
  }
  m_pending_events.emplace(event->m_arrive_time, event);
  if (is_reset && need_reset) {
    DebugLog << "need reset timer";
    resetArriveTime();
  }
}

void Timer::delTimerEvent(TimerEvent::ptr event) {
  event->m_is_cancled = true;
}

void Timer::resetArriveTime() {
  if (m_pending_events.size() == 0) {
    DebugLog << "no timerevent pending, size = 0";
    return;
  }

  int64_t now = getNowMs();
  auto it = m_pending_events.begin();
  if ((*it).first < now) {
    DebugLog<< "all timer events has already expire";
    return;
  }
  int64_t interval = (*it).first - now;

  itimerspec new_value;
  memset(&new_value, 0, sizeof(new_value));
  
  timespec ts;
  memset(&ts, 0, sizeof(ts));
  ts.tv_sec = interval * 1000;
  ts.tv_nsec = (interval % 1000) * 1000000;
  new_value.it_value = ts;

  int rt = timerfd_settime(m_fd, 0, &new_value, nullptr);

  if (rt != 0) {
    ErrorLog << "tiemr_settime error, interval=" << interval;
  } else {
    DebugLog << "reset timer succ";
  }

}

void Timer::onTimer() {
	int64_t now = getNowMs();
	auto it = m_pending_events.begin();
	std::vector<TimerEvent::ptr> tmps;
	std::vector<std::function<void()>> tasks;
	for (it = m_pending_events.begin(); it != m_pending_events.end(); ++it) {
		if ((*it).first <= now && !((*it).second->m_is_cancled)) {
			tmps.push_back((*it).second);
			tasks.push_back((*it).second->m_task);
		}	else {
			break;
		}
	}
	assert(m_reactor != nullptr);
	m_reactor->addTask(tasks);
	m_pending_events.erase(m_pending_events.begin(), it);
	for (auto i = tmps.begin(); i != tmps.end(); ++i) {
		if ((*i)->m_is_repeated) {
			(*i)->resetTime();
			addTimerEvent(*i, false);
		}
	}

	resetArriveTime();
}


}


