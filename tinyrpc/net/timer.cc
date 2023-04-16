#include <sys/timerfd.h>
#include <assert.h>
#include <time.h>
#include <string.h>
#include <vector>
#include <sys/time.h>
#include <functional>
#include <map>
#include "../comm/log.h"
#include "timer.h"
#include "mutex.h"
#include "fd_event.h"
#include "../coroutine/coroutine_hook.h"


extern read_fun_ptr_t g_sys_read_fun;  // sys read func

namespace tinyrpc {


int64_t getNowMs() {
  timeval val;
  gettimeofday(&val, nullptr);
  int64_t re = val.tv_sec * 1000 + val.tv_usec / 1000;
  return re;
}

Timer::Timer(Reactor* reactor) : FdEvent(reactor) {

  m_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK|TFD_CLOEXEC);
  DebugLog << "m_timer fd = " << m_fd;
  if (m_fd == -1) {
    DebugLog << "timerfd_create error";  
  }
  // DebugLog << "timerfd is [" << m_fd << "]";
	m_read_callback = std::bind(&Timer::onTimer, this);
  addListenEvents(READ);
  // updateToReactor();

}

Timer::~Timer() {
  unregisterFromReactor();
	close(m_fd);
}


void Timer::addTimerEvent(TimerEvent::ptr event, bool need_reset /*=true*/) {
  RWMutex::WriteLock lock(m_event_mutex);
  bool is_reset = false;
  if (m_pending_events.empty()) {
    is_reset = true;
  } else {
		auto it = m_pending_events.begin();
    if (event->m_arrive_time < (*it).second->m_arrive_time) {
      is_reset = true;
    }
  }
  m_pending_events.emplace(event->m_arrive_time, event);
  lock.unlock();

  if (is_reset && need_reset) {
    DebugLog << "need reset timer";
    resetArriveTime();
  }
  // DebugLog << "add timer event succ";
}

void Timer::delTimerEvent(TimerEvent::ptr event) {
  event->m_is_cancled = true;

  RWMutex::WriteLock lock(m_event_mutex);
  auto begin = m_pending_events.lower_bound(event->m_arrive_time);
  auto end = m_pending_events.upper_bound(event->m_arrive_time);
  auto it = begin;
  for (it = begin; it != end; it++) {
    if (it->second == event) {
      DebugLog << "find timer event, now delete it. src arrive time=" << event->m_arrive_time;
      break;
    }
  }
  if (it != m_pending_events.end()) {
    m_pending_events.erase(it);
  }
  lock.unlock();
  DebugLog << "del timer event succ, origin arrvite time=" << event->m_arrive_time;
}

void Timer::resetArriveTime() {
  RWMutex::ReadLock lock(m_event_mutex);
 	std::multimap<int64_t, TimerEvent::ptr> tmp = m_pending_events;
  lock.unlock();

  if (tmp.size() == 0) {
    DebugLog << "no timerevent pending, size = 0";
    return;
  }

  int64_t now = getNowMs();
  auto it = tmp.rbegin();
  if ((*it).first < now) {
    DebugLog<< "all timer events has already expire";
    return;
  }
  int64_t interval = (*it).first - now;

  itimerspec new_value;
  memset(&new_value, 0, sizeof(new_value));
  
  timespec ts;
  memset(&ts, 0, sizeof(ts));
  ts.tv_sec = interval / 1000;
  ts.tv_nsec = (interval % 1000) * 1000000;
  new_value.it_value = ts;

  int rt = timerfd_settime(m_fd, 0, &new_value, nullptr);

  if (rt != 0) {
    ErrorLog << "tiemr_settime error, interval=" << interval;
  } else {
    // DebugLog << "reset timer succ, next occur time=" << (*it).first;
  }

}

void Timer::onTimer() {

  // DebugLog << "onTimer, first read data";
  char buf[8];
  while(1) {
    if((g_sys_read_fun(m_fd, buf, 8) == -1) && errno == EAGAIN) {
      break;
    }
  }

  int64_t now = getNowMs();
  RWMutex::WriteLock lock(m_event_mutex);
	auto it = m_pending_events.begin();
	std::vector<TimerEvent::ptr> tmps;
  std::vector<std::pair<int64_t, std::function<void()>>> tasks;
	for (it = m_pending_events.begin(); it != m_pending_events.end(); ++it) {
		if ((*it).first <= now && !((*it).second->m_is_cancled)) {
			tmps.push_back((*it).second);
      tasks.push_back(std::make_pair((*it).second->m_arrive_time, (*it).second->m_task));
		}	else {
			break;
		}
	}

	m_pending_events.erase(m_pending_events.begin(), it);
  lock.unlock();

	for (auto i = tmps.begin(); i != tmps.end(); ++i) {
    // DebugLog << "excute timer event on " << (*i)->m_arrive_time;
		if ((*i)->m_is_repeated) {
			(*i)->resetTime();
			addTimerEvent(*i, false);
		}
	}

	resetArriveTime();

	// m_reactor->addTask(tasks);
  for (auto i : tasks) {
    // DebugLog << "excute timeevent:" << i.first;
    i.second();
  }
}

}


