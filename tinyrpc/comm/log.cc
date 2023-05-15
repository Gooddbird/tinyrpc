#include <time.h>
#include <sys/time.h>
#include <sstream>
#include <sys/syscall.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <algorithm>
#include <semaphore.h>
#include <errno.h>

#ifdef DECLARE_MYSQL_PLUGIN 
#include <mysql/mysql.h>
#endif


#include "tinyrpc/comm/log.h"
#include "tinyrpc/comm/config.h"
#include "tinyrpc/comm/run_time.h"
#include "tinyrpc/coroutine/coroutine.h"
#include "tinyrpc/net/reactor.h"
#include "tinyrpc/net/timer.h"




namespace tinyrpc {

extern tinyrpc::Logger::ptr gRpcLogger;
extern tinyrpc::Config::ptr gRpcConfig;


static std::atomic_int64_t g_rpc_log_index {0};
static std::atomic_int64_t g_app_log_index {0};

void CoredumpHandler(int signal_no) {
  ErrorLog << "progress received invalid signal, will exit";
  printf("progress received invalid signal, will exit\n");
  gRpcLogger->flush();
  pthread_join(gRpcLogger->getAsyncLogger()->m_thread, NULL);
  pthread_join(gRpcLogger->getAsyncAppLogger()->m_thread, NULL);

  signal(signal_no, SIG_DFL);
  raise(signal_no);
}

class Coroutine;

static thread_local pid_t t_thread_id = 0;
static pid_t g_pid = 0;

// LogLevel g_log_level = DEBUG;

pid_t gettid() {
  if (t_thread_id == 0) {
    t_thread_id = syscall(SYS_gettid);
  }
  return t_thread_id;
}

void setLogLevel(LogLevel level) {
  // g_log_level = level;
}


bool OpenLog() {
  if (!gRpcLogger) {
    return false;
  }
  return true;
}

LogEvent::LogEvent(LogLevel level, const char* file_name, int line, const char* func_name, LogType type)
  : m_level(level),
    m_file_name(file_name),
    m_line(line),
    m_func_name(func_name),
    m_type(type) {
}

LogEvent::~LogEvent() {

}

std::string levelToString(LogLevel level) {
  std::string re = "DEBUG";
  switch(level) {
    case DEBUG:
      re = "DEBUG";
      return re;
    
    case INFO:
      re = "INFO";
      return re;

    case WARN:
      re = "WARN";
      return re;

    case ERROR:
      re = "ERROR";
      return re;
    case NONE:
      re = "NONE";

    default:
      return re;
  }
}


LogLevel stringToLevel(const std::string& str) {
    if (str == "DEBUG")
      return LogLevel::DEBUG;
    
    if (str == "INFO")
      return LogLevel::INFO;

    if (str == "WARN")
      return LogLevel::WARN;

    if (str == "ERROR")
      return LogLevel::ERROR;

    if (str == "NONE")
      return LogLevel::NONE;

    return LogLevel::DEBUG;
}

std::string LogTypeToString(LogType logtype) {
  switch (logtype) {
    case APP_LOG:
      return "app";
    case RPC_LOG:
      return "rpc";
    default:
      return "";
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

  std::string s_level = levelToString(m_level);
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
  RunTime* runtime = getCurrentRunTime();
  if (runtime) {
    std::string msgno = runtime->m_msg_no;
    if (!msgno.empty()) {
      m_ss << "[" << msgno << "]\t";
    }

    std::string interface_name = runtime->m_interface_name;
    if (!interface_name.empty()) {
      m_ss << "[" << interface_name << "]\t";
    }

  }
  return m_ss;
}

void LogEvent::log() {
  m_ss << "\n";
  if (m_level >= gRpcConfig->m_log_level && m_type == RPC_LOG) {
    gRpcLogger->pushRpcLog(m_ss.str());
  } else if (m_level >= gRpcConfig->m_app_log_level && m_type == APP_LOG) {
    gRpcLogger->pushAppLog(m_ss.str());
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

Logger::~Logger() {
  flush();
  pthread_join(m_async_rpc_logger->m_thread, NULL);
  pthread_join(m_async_app_logger->m_thread, NULL);
}

void Logger::init(const char* file_name, const char* file_path, int max_size, int sync_inteval) {
  if (!m_is_init) {
    m_sync_inteval = sync_inteval;
    for (int i = 0 ; i < 1000000; ++i) {
      m_app_buffer.push_back("");
      m_buffer.push_back("");
    }
    // m_app_buffer.resize(1000000);
    // m_buffer.resize(1000000);

    m_async_rpc_logger = std::make_shared<AsyncLogger>(file_name, file_path, max_size, RPC_LOG);
    m_async_app_logger = std::make_shared<AsyncLogger>(file_name, file_path, max_size, APP_LOG);

    signal(SIGSEGV, CoredumpHandler);
    signal(SIGABRT, CoredumpHandler);
    signal(SIGTERM, CoredumpHandler);
    signal(SIGKILL, CoredumpHandler);
    signal(SIGINT, CoredumpHandler);
    signal(SIGSTKFLT, CoredumpHandler);

    // ignore SIGPIPE 
    signal(SIGPIPE, SIG_IGN);
    m_is_init = true;
  }
}

void Logger::start() {
  TimerEvent::ptr event = std::make_shared<TimerEvent>(m_sync_inteval, true, std::bind(&Logger::loopFunc, this));
  Reactor::GetReactor()->getTimer()->addTimerEvent(event);
}
	
void Logger::loopFunc() {
  std::vector<std::string> app_tmp;
  Mutex::Lock lock1(m_app_buff_mutex);
  app_tmp.swap(m_app_buffer);
  lock1.unlock();
  
  std::vector<std::string> tmp;
  Mutex::Lock lock2(m_buff_mutex);
  tmp.swap(m_buffer);
  lock2.unlock();

  m_async_rpc_logger->push(tmp);
  m_async_app_logger->push(app_tmp);
}

void Logger::pushRpcLog(const std::string& msg) {
  Mutex::Lock lock(m_buff_mutex);
  m_buffer.push_back(std::move(msg));
  lock.unlock();
}

void Logger::pushAppLog(const std::string& msg) {
  Mutex::Lock lock(m_app_buff_mutex);
  m_app_buffer.push_back(std::move(msg));
  lock.unlock();
}

void Logger::flush() {
  loopFunc();
  m_async_rpc_logger->stop();
  m_async_rpc_logger->flush();

  m_async_app_logger->stop();
  m_async_app_logger->flush();
}

AsyncLogger::AsyncLogger(const char* file_name, const char* file_path, int max_size, LogType logtype)
  : m_file_name(file_name), m_file_path(file_path), m_max_size(max_size), m_log_type(logtype) {
  int rt = sem_init(&m_semaphore, 0, 0);
  assert(rt == 0);

  rt = pthread_create(&m_thread, nullptr, &AsyncLogger::excute, this);
  assert(rt == 0);
  rt = sem_wait(&m_semaphore);
  assert(rt == 0);

}

AsyncLogger::~AsyncLogger() {

}

void* AsyncLogger::excute(void* arg) {
  AsyncLogger* ptr = reinterpret_cast<AsyncLogger*>(arg);
  int rt = pthread_cond_init(&ptr->m_condition, NULL);
  assert(rt == 0);

  rt = sem_post(&ptr->m_semaphore);
  assert(rt == 0);

  while (1) {
    Mutex::Lock lock(ptr->m_mutex);

    while (ptr->m_tasks.empty() && !ptr->m_stop) {
      pthread_cond_wait(&(ptr->m_condition), ptr->m_mutex.getMutex());
    }
    std::vector<std::string> tmp;
    tmp.swap(ptr->m_tasks.front());
    ptr->m_tasks.pop();
    bool is_stop = ptr->m_stop;
    lock.unlock();

    timeval now;
    gettimeofday(&now, nullptr);

    struct tm now_time;
    localtime_r(&(now.tv_sec), &now_time);

    const char *format = "%Y%m%d";
    char date[32];
    strftime(date, sizeof(date), format, &now_time);
    if (ptr->m_date != std::string(date)) {
      // cross day
      // reset m_no m_date
      ptr->m_no = 0;
      ptr->m_date = std::string(date);
      ptr->m_need_reopen = true;
    }

    if (!ptr->m_file_handle) {
      ptr->m_need_reopen = true;
    }    

    std::stringstream ss;
    ss << ptr->m_file_path << ptr->m_file_name << "_" << ptr->m_date << "_" << LogTypeToString(ptr->m_log_type) << "_" << ptr->m_no << ".log";
    std::string full_file_name = ss.str();

    if (ptr->m_need_reopen) {
      if (ptr->m_file_handle) {
        fclose(ptr->m_file_handle);
      }

      ptr->m_file_handle = fopen(full_file_name.c_str(), "a");
      if(ptr->m_file_handle == nullptr) {
        printf("open fail errno = %d reason = %s \n", errno, strerror(errno));
      }
      ptr->m_need_reopen = false;
    }

    if (ftell(ptr->m_file_handle) > ptr->m_max_size) {
      fclose(ptr->m_file_handle);

      // single log file over max size
      ptr->m_no++;
      std::stringstream ss2;
      ss2 << ptr->m_file_path << ptr->m_file_name << "_" << ptr->m_date << "_" << LogTypeToString(ptr->m_log_type) << "_" << ptr->m_no << ".log";
      full_file_name = ss2.str();

      // printf("open file %s", full_file_name.c_str());
      ptr->m_file_handle = fopen(full_file_name.c_str(), "a");
      ptr->m_need_reopen = false;
    }

    if (!ptr->m_file_handle) {
      printf("open log file %s error!", full_file_name.c_str());
    }

    for(auto i : tmp) {
      if (!i.empty()) {
        fwrite(i.c_str(), 1, i.length(), ptr->m_file_handle);
      }
    }
    tmp.clear();
    fflush(ptr->m_file_handle);
    if (is_stop) {
      break;
    }

  }
  if (!ptr->m_file_handle) {
    fclose(ptr->m_file_handle);
  }

  return nullptr;

}



void AsyncLogger::push(std::vector<std::string>& buffer) {
  if (!buffer.empty()) {
    Mutex::Lock lock(m_mutex);
    m_tasks.push(buffer);
    lock.unlock();
    pthread_cond_signal(&m_condition);
  }
}

void AsyncLogger::flush() {
  if (m_file_handle) {
    fflush(m_file_handle);
  }
}


void AsyncLogger::stop() {
  if (!m_stop) {
    m_stop = true;
    pthread_cond_signal(&m_condition);
  }
}

void Exit(int code) {
  #ifdef DECLARE_MYSQL_PLUGIN
  mysql_library_end();
  #endif

  printf("It's sorry to said we start TinyRPC server error, look up log file to get more deatils!\n");
  gRpcLogger->flush();
  pthread_join(gRpcLogger->getAsyncLogger()->m_thread, NULL);

  _exit(code);
}


}
