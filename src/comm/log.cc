#include <time.h>
#include <sys/time.h>
#include <sstream>
#include <sys/syscall.h>
#include <unistd.h>
#include <iostream>
#include <stdio.h>


#include "log.h"
#include "../coroutine/coroutine.h"
#include "../net/reactor.h"
#include "../net/timer.h"


extern tinyrpc::Logger* gRpcLogger;

namespace tinyrpc {

class Coroutine;

static thread_local pid_t t_thread_id = 0;
static pid_t g_pid = 0;

LogLevel g_log_level = DEBUG;

pid_t gettid() {
  if (t_thread_id == 0) {
    t_thread_id = syscall(SYS_gettid);
  }
  return t_thread_id;
}

void setLogLevel(LogLevel level) {
  g_log_level = level;
}


LogEvent::LogEvent(LogLevel level, const char* file_name, int line, const char* func_name)
  : m_level(level),
    m_file_name(file_name),
    m_line(line),
    m_func_name(func_name) {
}

LogEvent::~LogEvent() {

}

void levelToString(LogLevel level, std::string& re) {
  switch(level) {
    case DEBUG:
      re = "DEBUG";
      return;
    
    case INFO:
      re = "INFO";
      return;

    case WARN:
      re = "WARN";
      return;

    case ERROR:
      re = "ERROR";
      return;

    default:
      re = "";
      return;
  }
}


std::stringstream& LogEvent::getStringStream() {

  // time_t now_time = m_timestamp;
  
  gettimeofday(&m_timeval, nullptr);

  struct tm time; 
  localtime_r(&(m_timeval.tv_sec), &time);

  const char* format = "%Y-%m-%d %H:%M:%S";
  char buf[128];
  strftime(buf, sizeof(buf), format, &time);

  m_ss << "[" << buf << "." << m_timeval.tv_usec << "]\t"; 

  std::string s_level;
  levelToString(m_level, s_level);
  m_ss << "[" << s_level << "]\t";

  if (g_pid == 0) {
    g_pid = getpid();
  }
  m_pid = g_pid;  

  if (t_thread_id == 0) {
    t_thread_id = gettid();
  }
  m_tid = t_thread_id;

  m_cor_id = Coroutine::GetCurrentCoroutine()->getCorId();
  
  m_ss << "[" << m_pid << "]\t" 
		<< "[" << m_tid << "]\t"
		<< "[" << m_cor_id << "]\t"
    << "[" << m_file_name << ":" << m_line << "]\t";
    // << "[" << m_func_name << "]\t";
  
  return m_ss;
}

void LogEvent::log() {
	m_ss << "\n";
  if (m_level >= g_log_level) {

    // printf("========%s", m_ss.str().c_str());
    gRpcLogger->push(m_ss.str());
  }
}


LogTmp::LogTmp(LogEvent::ptr event) : m_event(event) {

}

std::stringstream& LogTmp::getStringStream() {
  return m_event->getStringStream();
}

LogTmp::~LogTmp() {
  m_event->log(); 
}

Logger::Logger() {
  // cannot do anything which will call LOG ,otherwise is will coredump

}

void Logger::init() {
  TimerEvent::ptr event = std::make_shared<TimerEvent>(1000, true, std::bind(&Logger::loopFunc, this));
  Reactor::GetReactor()->getTimer()->addTimerEvent(event);
  m_async_logger = std::make_shared<AsyncLogger>();
}
	
void Logger::loopFunc() {
  std::vector<std::string> tmp;
  Mutex::Lock lock(m_mutex);
  tmp.swap(m_buffer);
  lock.unlock();

  m_async_logger->push(tmp);
}

void Logger::push(const std::string& msg) {

  Mutex::Lock lock(m_mutex);
  m_buffer.push_back(msg);
  lock.unlock();
}

AsyncLogger::AsyncLogger() {
  pthread_create(&m_thread, nullptr, &AsyncLogger::excute, this);
}

AsyncLogger::~AsyncLogger() {

}

void* AsyncLogger::excute(void* arg) {
  AsyncLogger* ptr = reinterpret_cast<AsyncLogger*>(arg);
  pthread_cond_init(&ptr->m_condition, NULL);

  while (1) {
    Mutex::Lock lock(ptr->m_mutex);

    while (ptr->m_tasks.empty()) {
      pthread_cond_wait(&(ptr->m_condition), ptr->m_mutex.getMutex());
    }
    std::vector<std::string> tmp;
    tmp.swap(ptr->m_tasks.front());
    ptr->m_tasks.pop();
    lock.unlock();

    for(auto i : tmp) {
      printf(i.c_str());
    }

  }

  return nullptr;

}


void AsyncLogger::push(std::vector<std::string>& buffer) {
  Mutex::Lock lock(m_mutex);
  m_tasks.push(buffer);
  lock.unlock();
  pthread_cond_broadcast(&m_condition);

}

}
