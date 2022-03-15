# TinyRpc
Use c++ to make a tiny rpc framework.

## 1. How to Build

1. git clone
2. cd tinyrpc
3. mkdir build; mkdir bin;
4. cd build
5. cmake ..
6. make
7. cd ../bin
8. ./test_xxxx

## 2. Design of RPC
#### First design a simple rpc protocal:
(It reference to an acticle [chenshuo: https://www.cnblogs.com/Solstice/archive/2011/04/03/2004458.html])

```c++
char* start;    // 0x02
int32_t pk_len;
int32_t service_full_name_len;
std::string service_full_name;
pb binary data;
int32_t checknum;
char* end;      // 0x03
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

pk_len = 2* sizeof(char*) + 3 * sizeof(int32_t) + service_name.length() + pb_binary_data.length();

ss << 0x02 << pk_len(to net byte order) << sizeof(service_name)(to net byte order) << service_name << pb_binary_data << checksum(to net byte order) << 0x03;
```
- **Notice**: All integer parameters will be transform to **net byte order**(**big endian byte order**) !!!



## Reference
libco: https://github.com/Tencent/libco

sylar: https://github.com/sylar-yin/sylar

muduo: https://github.com/chenshuo/muduo


