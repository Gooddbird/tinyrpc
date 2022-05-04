# TinyRPC
TinyRPC 是一款使用 C++11 开发的小型 RPC 框架。TinyRPC 是本人在学习过程的作品，这些代码没有达到工业强度，也可能存在一些未知 BUG。读者请自行辨别。

麻雀虽小五脏俱全，从命名上就能看出来，TinyRPC 框架主要用义是为了让读者能快速地、轻量化地搭建出具有一定性能的 RPC 服务。我不敢说有多高的性能，但至少能应付目前大多数场景了。

TinyRPC 没有跨平台，只支持 LINUX 系统，并且必须是 64 位的系统，因为协程切换只实现了 64 位系统的，而没有兼容 32 位系统。

TinyRPC 的自定义的 RPC 协议报文暂时只支持基于 PROTOBUF 的序列化。

TinyRPC 框架的主要模块包括：日志封装、协程封装、Reactor封装、Tcp 封装、自定义协议封装、以及RPC封装等。

## 1. 如何使用

1. git clone
2. cd tinyrpc
3. mkdir bin & mkdir lib;
4. make

make 完成后，会在 lib 目录下生成静态库文件 libtinyrpc.a。在真正开发 RPC 服务时，只需要静态链接这个库，就能使用其中的函数了。(当然也能动态链接, 读者可自行修改 makefile 满足自己的需求)。
更多详细信息请参考这个仓库：

此外，还会在 bin 目录下生成一些单元测试文件。运行 bin/test_xxxx 文件，可以简单测试下 TinyRPC 的一些单元模块功能是否正常。


## 2. Design of RPC
#### First design a simple rpc protocal:
(It reference to an acticle [chenshuo: https://www.cnblogs.com/Solstice/archive/2011/04/03/2004458.html])

```c++
/*
**  min of package is: 1 + 4 + 4 + 4 + 4 + 4 + 4 + 1 = 26 bytes
**
*/
char start;                         // indentify start of a TinyPb protocal data
int32_t pk_len {0};                 // len of all package(include start char and end char)
int32_t msg_req_len {0};            // len of msg_req
std::string msg_req;                // msg_req, which identify a request, such as '1089988112457683520'
int32_t service_name_len {0};       // len of service full name
std::string service_full_name;      // service full name, like QueryService.query_name
int32_t err_code {0};               // err_code, 0 -- call rpc success, otherwise -- call rpc failed. it only be seted by RpcController
int32_t err_info_len {0};           // len of err_info
std::string err_info;               // err_info, empty -- call rpc success, otherwise -- call rpc failed, it will display details of reason why call rpc failed. it only be seted by RpcController
std::string pb_data {"1"};          // business pb data
int32_t check_num {-1};             // check_num of all package. to check legality of data
char end;                           // identify end of a TinyPb protocal data
```
- **Notice**: **pk_len** is the length of all package(including **[strat]** and **[end]**)

For example, define a proto:
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
So, it will be encode like this progress:

```c++
stringstream ss;
QueryReq req;
req.set_id(1);
pb_binary_data = req.serilizeToString();
service_name = "QueryService.query_name";

pk_len = 2* sizeof(char*) + 6 * sizeof(int32_t) + service_name.length() + pb_binary_data.length() + msg_req.length() + err_info.length();

ss << 0x02 << pk_len(to net byte order) << msg_req_len(net byte order) << msg_req << sizeof(service_name)(to net byte order) << service_name << err_code << err_info_len << err_info << pb_binary_data << checksum(to net byte order) << 0x03;
```
- **Notice**: All integer parameters will be transform to **net byte order**(**big endian byte order**) !!!



## 参考资料
libco: https://github.com/Tencent/libco
sylar: https://github.com/sylar-yin/sylar
muduo: https://github.com/chenshuo/muduo


