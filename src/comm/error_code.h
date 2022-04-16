#ifndef TINYRPC_COMM_ERRORCODE_H
#define TINYRPC_COMM_ERRORCODE_H


namespace tinyrpc {

#ifndef SYS_ERROR_PREFIX
#define SYS_ERROR_PREFIX(xx) 1000##xx
#endif // SYS_ERROR_PREFIX(xx)

//
// client error
// 
const int ERROR_FAILED_CONNECT = SYS_ERROR_PREFIX(0001);      // failed to connection peer host
const int ERROR_FAILED_GET_REPLY = SYS_ERROR_PREFIX(0002);    // failed to get server reply

 
} // namespace tinyrpc 


#endif // TINYRPC_COMM_ERRORCODE_H