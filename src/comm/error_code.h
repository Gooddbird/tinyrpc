#ifndef TINYRPC_COMM_ERRORCODE_H
#define TINYRPC_COMM_ERRORCODE_H


namespace tinyrpc {

#ifndef SYS_ERROR_PREFIX
#define SYS_ERROR_PREFIX(xx) 1000##xx
#endif // SYS_ERROR_PREFIX(xx)

//
// error
// 
const int ERROR_FAILED_CONNECT = SYS_ERROR_PREFIX(0001);        // failed to connection peer host
const int ERROR_FAILED_GET_REPLY = SYS_ERROR_PREFIX(0002);      // failed to get server reply
const int ERROR_FAILED_DESERIALIZE = SYS_ERROR_PREFIX(0003);    // deserialize failed
const int ERROR_FAILED_SERIALIZE = SYS_ERROR_PREFIX(0004);      // serialize failed

const int ERROR_FAILED_ENCODE = SYS_ERROR_PREFIX(0005);      // encode failed
const int ERROR_FAILED_DECODE = SYS_ERROR_PREFIX(0006);      // decode failed

 
} // namespace tinyrpc 


#endif // TINYRPC_COMM_ERRORCODE_H