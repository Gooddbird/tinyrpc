#ifndef TINYRPC_NET_NET_ADDRESS_H
#define TINYRPC_NET_NET_ADDRESS_H

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>



namespace tinyrpc {


class NetAddress {

 public:
  
  virtual sockaddr* getSockAddr();

 private:

};


class IPAddress {

 public:
  IPAddress(std::string ip, uint16_t port);

  sockaddr* getSockAddr();

 private:
  
  std::string m_ip;
  uint16_t m_port;
  sockaddr_in m_addr;


};


}


#endif

