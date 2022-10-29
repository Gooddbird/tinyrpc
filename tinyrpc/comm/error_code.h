#ifndef TINYRPC_COMM_ERRORCODE_H
#define TINYRPC_COMM_ERRORCODE_H


namespace tinyrpc {

#ifndef SYS_ERROR_PREFIX
#define SYS_ERROR_PREFIX(xx) 1000##xx
#endif // SYS_ERROR_PREFIX(xx)

//
// error
// 
const int ERROR_PEER_CLOSED = SYS_ERROR_PREFIX(0000);           // connect when peer close
const int ERROR_FAILED_CONNECT = SYS_ERROR_PREFIX(0001);        // failed to connection peer host
const int ERROR_FAILED_GET_REPLY = SYS_ERROR_PREFIX(0002);      // failed to get server reply
const int ERROR_FAILED_DESERIALIZE = SYS_ERROR_PREFIX(0003);    // deserialize failed
const int ERROR_FAILED_SERIALIZE = SYS_ERROR_PREFIX(0004);      // serialize failed

const int ERROR_FAILED_ENCODE = SYS_ERROR_PREFIX(0005);      // encode failed
const int ERROR_FAILED_DECODE = SYS_ERROR_PREFIX(0006);      // decode failed

const int ERROR_RPC_CALL_TIMEOUT = SYS_ERROR_PREFIX(0007);    // call rpc timeout

const int ERROR_SERVICE_NOT_FOUND = SYS_ERROR_PREFIX(0008);    // not found service name

const int ERROR_METHOD_NOT_FOUND = SYS_ERROR_PREFIX(0009);    // not found method 

const int ERROR_PARSE_SERVICE_NAME = SYS_ERROR_PREFIX(0010);    // not found service name
const int ERROR_NOT_SET_ASYNC_PRE_CALL = SYS_ERROR_PREFIX(0011);            // you didn't set some nessary param before call async rpc
const int ERROR_CONNECT_SYS_ERR = SYS_ERROR_PREFIX(0012);           // connect sys error
 
} // namespace tinyrpc 


#endif // TINYRPC_COMM_ERRORCODE_H