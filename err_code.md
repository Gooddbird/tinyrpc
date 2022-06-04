# TinyPB 协议 ErrorCode 说明文档

## 1.概述
TinyPB 协议使用错误码来标识 RPC 调用过程的那些不可控的错误。这些错误码是框架级错误码，当出现这些错误码时，说明是 RPC 调用的链路出了问题。自然，这次 RPC 调用是失败的。

一般来说，在调用 RPC 时，需要判断两个错误码，例如：
```c++
stub.query_name(&rpc_controller, &rpc_req, &rpc_res, NULL);

if (rpc_controller.ErrorCode() != 0) {
  ErrorLog << "failed to call QueryServer rpc server";
  char buf[512];
  sprintf(buf, html, "failed to call QueryServer rpc server");
  setHttpBody(res, std::string(buf));
  return;
}

if (rpc_res.ret_code() != 0) {
  std::stringstream ss;
  ss << "QueryServer rpc server return bad result, ret = " << rpc_res.ret_code() << ", and res_info = " << rpc_res.res_info();
  ErrorLog << ss.str();
  char buf[512];
  sprintf(buf, html, ss.str().c_str());
  setHttpBody(res, std::string(buf));
  return;
}
```

rpc_controller.ErrorCode 是 RPC **框架级错误码**，即这个文档里面锁描述的东西。该错误码的枚举值已经被定义好如下表格，一般情况下不会变更。当此错误码不为0时，请检查 RPC 通信链路是否有问题，网络连接是否有异常。当然，TinyPB 协议里面的 err_info 字段也会详细的描述错误信息。

另一个错误码是**业务错误码**，通常他被定义在 RPC 方法返回结构体的第一个字段中。出现这个错误码一般是对端在进行业务处理时出现了非预期的结果，此时将返回对应的错误码和错误信息。这个错误码的枚举值应由 RPC 通信双方自行约定。

## 2. 错误码详细对应文档
err_code 详细说明如下表：

|  **错误码** | **错误代码** | **错误码描述** |
|  ----  | ----  | ---- |
| ERROR_PEER_CLOSED | 10000000 | connect 时对端关闭，一般是对端没有进程在监听此端口 |
| ERROR_FAILED_CONNECT | 10000001 | connect 失败|
| ERROR_FAILED_GET_REPLY | 10000002 | RPC 调用未收到对端回包数据 |
| ERROR_FAILED_DESERIALIZE | 10000003 | 反序列化失败，这种情况一般是 TinyPb 里面的 pb_data 有问题 |
| ERROR_FAILED_SERIALIZE | 10000004 | 序列化失败|
| ERROR_FAILED_ENCODE | 10000005 | 编码失败 |
| ERROR_FAILED_DECODE | 10000006 |  解码失败|
| ERROR_RPC_CALL_TIMEOUT | 10000007 | 调用 RPC 超时, 这种情况请检查下 RPC 的超时时间是否太短 |
| ERROR_SERVICE_NOT_FOUND | 10000008 | Service 不存在，即对方没有注册这个 Service |
| ERROR_METHOD_NOT_FOUND | 10000009 | Method 不存在，对方没有这个 方法|
| ERROR_PARSE_SERVICE_NAME | 10000010 | 解析 service_name 失败|