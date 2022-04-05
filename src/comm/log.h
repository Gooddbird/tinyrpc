#ifndef TINYRPC_LOG_LOG_H
#define TINYRPC_LOG_LOG_H

#include <sstream>
#include <sstream>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <unistd.h>
#include <memory>
#include "../net/mutex.h"


namespace tinyrpc {

enum LogLevel {
	DEBUG = 1,
	INFO = 2,
	WARN = 3,
	ERROR = 4
};

extern LogLevel g_log_level;

#define DebugLog \
	if (tinyrpc::LogLevel::DEBUG >= tinyrpc::g_log_level) \
		tinyrpc::LogTmp(tinyrpc::LogEvent::ptr(new tinyrpc::LogEvent(tinyrpc::LogLevel::DEBUG, __FILE__, __LINE__, __func__))).getStringStream()

#define InfoLog \
	if (tinyrpc::LogLevel::INFO >= tinyrpc::g_log_level) \
		tinyrpc::LogTmp(tinyrpc::LogEvent::ptr(new tinyrpc::LogEvent(tinyrpc::LogLevel::INFO, __FILE__, __LINE__, __func__))).getStringStream()

#define WarnLog \
	if (tinyrpc::LogLevel::WARN >= tinyrpc::g_log_level) \
		tinyrpc::LogTmp(tinyrpc::LogEvent::ptr(new tinyrpc::LogEvent(tinyrpc::LogLevel::WARN, __FILE__, __LINE__, __func__))).getStringStream()

#define ErrorLog \
	if (tinyrpc::LogLevel::ERROR >= tinyrpc::g_log_level) \
		tinyrpc::LogTmp(tinyrpc::LogEvent::ptr(new tinyrpc::LogEvent(tinyrpc::LogLevel::ERROR, __FILE__, __LINE__, __func__))).getStringStream()


void setLogLevel(LogLevel level);

pid_t gettid();

class LogEvent {

 public:
 	
	typedef std::shared_ptr<LogEvent> ptr;
	LogEvent(LogLevel level, const char* file_name, int line, const char* func_name);

	~LogEvent();

	std::stringstream& getStringStream();

	void log();


 private:
		
	const char* m_msg_no;
	// uint64_t m_timestamp;
	timeval m_timeval;
	LogLevel m_level;
	pid_t m_pid {0};
	pid_t m_tid {0};
	int m_cor_id {0};

	const char* m_file_name;
	int m_line;
	const char* m_func_name;

	std::stringstream m_ss;

 	Mutex m_mutex;

};


class LogTmp {
 
 public:
	explicit LogTmp(LogEvent::ptr event);

	~LogTmp();

	std::stringstream& getStringStream();

 private:
	LogEvent::ptr m_event;

};


class LogPrinter {

 public:
	
	void log();

 private:

};

}


#endif
