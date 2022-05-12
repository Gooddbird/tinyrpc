# TinyPb 协议详解

## 1. TinyPb 协议报文格式
**TinyPb** 协议包报文用 c++ 伪代码描述如下：
```c++
/*
**  min of package is: 1 + 4 + 4 + 4 + 4 + 4 + 4 + 1 = 26 bytes
**
*/
char start;                         // 代表报文的开始， 一般是 0x02
int32_t pk_len {0};                 // 整个包长度，单位 byte
int32_t msg_req_len {0};            // msg_req 字符串长度
std::string msg_req;                // msg_req,标识一个 rpc 请求或响应。 一般来说 请求 和 响应使用同一个 msg_req.
int32_t service_name_len {0};       // service_name 长度
std::string service_full_name;      // 完整的 rpc 方法名， 如 QueryService.query_name
int32_t err_code {0};               // 框架级错误代码. 0 代表调用正常，非 0 代表调用失败
int32_t err_info_len {0};           // err_info 长度
std::string err_info;               // 详细错误信息， err_code 非0时会设置该字段值
std::string pb_data;                // 业务 protobuf 数据，由 google 的 protobuf 序列化后得到
int32_t check_num {0};             // 包检验和，用于检验包数据是否有损坏
char end;                           // 代表报文结束，一般是 0x03
```

注释信息已经很完整了。另外几个需要特殊说明的字段如下：

**err_code**: err_code 是框架级别的错误码，即代表调用 RPC 过程中发生的错误，如对端关闭、调用超时等。err_code 为0 代表此次 RPC 调用正常，即正常发送数据且接收到回包。非 0 值代表调用失败，此时会设置 err_info 为详细的错误信息。
关于错误码更多详细错误信息可见文档：[err_cdoe.md](./err_code.md).

**service_full_name** : 是指的调用的完整方法名。即 servicename.methodname。一般来说，一个 **TinyRPC** 服务需要注册一个 **Service** (这里的 Service 指的继承了google::protobuf::Service 的类)，而一个 Service 下包含多个方法。

**pk_len**: pk_len 代表整个协议包的长度，单位是1字节，且包括 **[strat]** 字符 和 **[end]** 字符。

**TinyPb** 协议报文中包含了多个 len 字段，这主要是为了用空间换时间，接收方在提前知道长度的情况下，更方便解码各个字段，从而提升了 decode 效率。

另外，**TinyPb** 协议里面所有的 int 类型的字段在编码时都会先转为**网络字节序**！

## 2. TinyPb 编码示例

首先定义一个 protobuf 文件:
**QueryService.proto**

```c++
message QueryReq {
  int32 req_no = 1;
  int32 id = 2;
}

message QueryNameRes {
  int32 ret_code = 1;
  string res_info = 2;
  int32 id = 3;
  string name = 4;
}

message QueryAgeRes {
  int32 ret_code = 1;
  string res_info = 2;
  int32 id = 3;
  int32 age = 4;
}

service QueryService {
  rpc query_name(QueryReq) returns (QueryNameRes);
  rpc query_age(QueryReq) returns (QueryAgeRes);
}
```
那么使用 TinyPb 协议编码过程如下：

```c++
stringstream ss;
QueryReq req;
req.set_id(1);
pb_binary_data = req.serilizeToString();
service_name = "QueryService.query_name";

pk_len = 2* sizeof(char*) + 6 * sizeof(int32_t) + service_name.length() + pb_binary_data.length() + msg_req.length() + err_info.length();

ss << 0x02 << pk_len(to net byte order) << msg_req_len(net byte order) << msg_req << sizeof(service_name)(to net byte order) << service_name << err_code << err_info_len << err_info << pb_binary_data << checksum(to net byte order) << 0x03;
```