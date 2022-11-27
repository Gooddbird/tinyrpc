#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <sstream>
#include "net_address.h"
#include "../comm/log.h"

namespace tinyrpc {

bool IPAddress::CheckValidIPAddr(const std::string& addr) {
  size_t i = addr.find_first_of(":");
  if (i == addr.npos) {
    return false;
  }
  int port = std::atoi(addr.substr(i + 1, addr.size() - i - 1).c_str());
  if (port < 0 || port > 65536) {
    return false;
  }

  if(inet_addr(addr.substr(0, i).c_str()) == INADDR_NONE) {
    return false;
  }
  return true;
}

IPAddress::IPAddress(const std::string& ip, uint16_t port) 
  : m_ip(ip), m_port(port) {
  
  memset(&m_addr, 0, sizeof(m_addr));
  m_addr.sin_family = AF_INET;
  m_addr.sin_addr.s_addr = inet_addr(m_ip.c_str());
  m_addr.sin_port = htons(m_port);
 
  DebugLog << "create ipv4 address succ [" << toString() << "]";
}

IPAddress::IPAddress(sockaddr_in addr) : m_addr(addr) {
  // if (m_addr.sin_family != AF_INET) {
    // ErrorLog << "err family, this address is valid";
  // }
  DebugLog << "ip[" << m_ip << "], port[" << addr.sin_port;
  m_ip = std::string(inet_ntoa(m_addr.sin_addr));
  m_port = ntohs(m_addr.sin_port);
}

IPAddress::IPAddress(const std::string& addr) {
  size_t i = addr.find_first_of(":");
  if (i == addr.npos) {
    ErrorLog << "invalid addr[" << addr << "]";
    return;
  }
  m_ip = addr.substr(0, i);
  m_port = std::atoi(addr.substr(i + 1, addr.size() - i - 1).c_str());

  memset(&m_addr, 0, sizeof(m_addr));
  m_addr.sin_family = AF_INET;
  m_addr.sin_addr.s_addr = inet_addr(m_ip.c_str());
  m_addr.sin_port = htons(m_port);
  DebugLog << "create ipv4 address succ [" << toString() << "]";

}

IPAddress::IPAddress(uint16_t port) : m_port(port) {
  memset(&m_addr, 0, sizeof(m_addr));
  m_addr.sin_family = AF_INET;
  m_addr.sin_addr.s_addr = INADDR_ANY;
  m_addr.sin_port = htons(m_port);
 
  DebugLog << "create ipv4 address succ [" << toString() << "]";
}

int IPAddress::getFamily() const {
  return m_addr.sin_family;
}

sockaddr* IPAddress::getSockAddr() {
  return reinterpret_cast<sockaddr*>(&m_addr);
}

std::string IPAddress::toString() const {
  std::stringstream ss;
  ss << m_ip << ":" << m_port;
  return ss.str();
}

socklen_t IPAddress::getSockLen() const {
  return sizeof(m_addr);
}


UnixDomainAddress::UnixDomainAddress(std::string& path) : m_path(path) {

  memset(&m_addr, 0, sizeof(m_addr));
  unlink(m_path.c_str());
  m_addr.sun_family = AF_UNIX;
  strcpy(m_addr.sun_path, m_path.c_str());

}
UnixDomainAddress::UnixDomainAddress(sockaddr_un addr) : m_addr(addr) {
  m_path = m_addr.sun_path; 
}

int UnixDomainAddress::getFamily() const {
  return m_addr.sun_family;
}

sockaddr* UnixDomainAddress::getSockAddr() {
  return reinterpret_cast<sockaddr*>(&m_addr);
}

socklen_t UnixDomainAddress::getSockLen() const {
  return sizeof(m_addr);
}

std::string UnixDomainAddress::toString() const {
  return m_path;
}

}
