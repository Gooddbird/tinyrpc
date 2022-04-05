#include <sys/socket.h>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include "../src/comm/log.h"
#include "../src/coroutine/coroutine_hook.h"
#include "../src/net/reactor.h"
#include "../src/coroutine/coroutine.h"
#include "../src/coroutine/coroutine_pool.h"
#include "../src/net/tinypb/tinypb_codec.h"
#include "../src/net/tinypb/tinypb_data.h"
#include "tinypb.pb.h"
#include "google/protobuf/message.h"


int connfd = -1;

void connect_co() {
  // int count = 10000;
  // while(count--) {
  connfd = socket(AF_INET, SOCK_STREAM, 0);

  if (connfd == -1) {
    ErrorLog << "socket error, fd=-1, error=" << strerror(errno);
  }
  sockaddr_in ser_addr;
  ser_addr.sin_family = AF_INET;

  ser_addr.sin_port = htons(39999); 
  ser_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  int rt = connect_hook(connfd, reinterpret_cast<sockaddr*>(&ser_addr), sizeof(ser_addr));
  DebugLog << "rt " << rt;
  if (rt == -1) {
    sleep(1);
    DebugLog << "connect error";
  } else if (rt == 0) {
    DebugLog << "connect succ";
  } else {
    DebugLog << "unknown rt " << rt;
  }
  while(1) {
    int a;
    DebugLog << "input in integer to set counts that send protobuf data";
    std::cin >> a;

    // char buf[20];
    // buf[0] = 0x02;
    // int32_t pk_len = 20;
    // pk_len = htonl(pk_len);

    // memcpy(&buf[1], &pk_len, 4);
    // int sergvice_name_len = 6;
    // sergvice_name_len = htonl(sergvice_name_len);

    // memcpy(&buf[5], &sergvice_name_len, 4);
    // char service_name[6] = "ikerl";
    // memcpy(&buf[9], &service_name[0], 6);
    // int checksum = 1;
    // memcpy(&buf[15], &checksum, 4);
    // buf[19] = 0x03;

    queryAgeReq req;
    req.set_id(9876); 
    tinyrpc::TinyPbStruct pb_struct;
    pb_struct.service_full_name = "QueryService.query_name";
    req.SerializeToString(&(pb_struct.pb_data));

    DebugLog << "pb size = " << pb_struct.pb_data.size();

    queryAgeReq req_test;
    if (!req_test.ParseFromString(pb_struct.pb_data))
    {
      ErrorLog << "parse request error";
      return;
    }
    DebugLog << "req.id = " << req_test.id(); 

    tinyrpc::TinyPbCodeC m_codec;
    int len = 0;
    const char* buf = m_codec.encodePbData(&pb_struct, len);

		// char buf[4] = {'a', 'b', 'c', 'd'};
    int rt = 0;
    while (a--) {
      
      rt = write(connfd, buf, len);
      DebugLog << "[seq: "<< a << "]succ write[" << pb_struct.service_full_name << "], write count=" << rt << ", src count=" << len;
    }
  }
 
}


int main(int argc, char* argv[]) {
  
  DebugLog << "main begin";
  // tinyrpc::enableHook();
  tinyrpc::Coroutine::GetCurrentCoroutine();
  tinyrpc::Coroutine::ptr cor = std::make_shared<tinyrpc::Coroutine>(128 * 1024, connect_co);
  tinyrpc::Reactor* reactor = tinyrpc::Reactor::GetReactor(); 
  tinyrpc::Coroutine::Resume(cor.get());
  reactor->loop();

  return 0;
 
}
