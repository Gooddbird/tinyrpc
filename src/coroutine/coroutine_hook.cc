#include <assert.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>
#include "coroutine_hook.h"
#include "coroutine.h"
#include "../net/fd_event.h"
#include "../net/reactor.h"
#include "../log/log.h"

#define HOOK_SYS_FUNC(name) static name##_fun_ptr_t g_sys_##name##_fun = (name##_fun_ptr_t)dlsym(RTLD_NEXT, #name);


HOOK_SYS_FUNC(accept);
HOOK_SYS_FUNC(read);

static int g_hook_enable = false;

extern "C" {

void toEpoll() {


}

ssize_t read(int fd, void *buf, size_t count) {
	DebugLog << "this is hook read";
  if (!g_hook_enable) {
    DebugLog << "hook disable, call sys func";
    return g_sys_read_fun(fd, buf, count);
  }
	tinyrpc::Reactor* reactor = tinyrpc::Reactor::GetReactor();
	assert(reactor != nullptr);

	tinyrpc::FdEvent::ptr fd_event = std::make_shared<tinyrpc::FdEvent>(reactor, fd);
	fd_event->setNonBlock();
	
	tinyrpc::Coroutine::ptr cur_cor;
	cur_cor.reset(tinyrpc::Coroutine::GetCurrentCoroutine());

	fd_event->setCallBack(tinyrpc::IOEvent::READ, 
		[cur_cor]() {
			tinyrpc::Coroutine::Resume(cur_cor);
		}
	);

	fd_event->addListenEvents(tinyrpc::IOEvent::READ);
	fd_event->updateToReactor();

	DebugLog << "read func to yield";
	tinyrpc::Coroutine::Yield();

	DebugLog << "read func yield back, now to call sys read";
	return g_sys_read_fun(fd, buf, count);

}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
	DebugLog << "this is hook accept";
  if (!g_hook_enable) {
    DebugLog << "hook disable, call sys func";
    return g_sys_accept_fun(sockfd, addr, addrlen);
  }
	tinyrpc::Reactor* reactor = tinyrpc::Reactor::GetReactor();
	assert(reactor != nullptr);

	tinyrpc::FdEvent::ptr fd_event = std::make_shared<tinyrpc::FdEvent>(reactor, sockfd);
	fd_event->setNonBlock();
	
	tinyrpc::Coroutine::ptr cur_cor;
	cur_cor.reset(tinyrpc::Coroutine::GetCurrentCoroutine());

	fd_event->setCallBack(tinyrpc::IOEvent::READ, 
		[cur_cor]() {
			tinyrpc::Coroutine::Resume(cur_cor);
		}
	);

	fd_event->addListenEvents(tinyrpc::IOEvent::READ);
	fd_event->updateToReactor();

	DebugLog << "accept func to yield";
	tinyrpc::Coroutine::Yield();

	DebugLog << "accept func yield back, now to call sys accept";
	return g_sys_accept_fun(sockfd, addr, addrlen);

}

ssize_t write(int fd, const void *buf, size_t count) {
	HOOK_SYS_FUNC(write);
	return g_sys_write_fun(fd, buf, count);
}

typedef int (*connect_fun_ptr_t)(int sockfd, const struct sockaddr *addr, socklen_t addrlen);


typedef int (*socket_fun_ptr_t)(int domain, int type, int protocol);

}


namespace tinyrpc {

void enableHook() {
  g_hook_enable = true;
}

void disabkeHook() {
  g_hook_enable = false;
}

}






