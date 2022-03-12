#ifndef TINYRPC_COROUTINE_COUROUTINE_HOOK_H
#define TINYRPC_COROUTINE_COUROUTINE_HOOK_H

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>

namespace tinyrpc {
	void enableHook();

	void disableHook();
}


typedef ssize_t (*read_fun_ptr_t)(int fd, void *buf, size_t count);

typedef ssize_t (*write_fun_ptr_t)(int fd, const void *buf, size_t count);

typedef int (*connect_fun_ptr_t)(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

typedef int (*accept_fun_ptr_t)(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

typedef int (*socket_fun_ptr_t)(int domain, int type, int protocol);


extern "C" {

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

ssize_t read(int fd, void *buf, size_t count);


// set max timeout, s
void setMaxTimeOut (int v);

}


#endif
