#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <sstream>
#include "net_address.h"
#include "../log/log.h"

namespace tinyrpc {

IPAddress::IPAddress(const std::string& ip, uint16_t port) 
  : m_ip(ip), m_port(port) {
  
  memset(&m_addr, 0, sizeof(m_addr));
  m_addr.sin_family = AF_INET;
  m_addr.sin_addr.s_addr = inet_addr(m_ip.c_str());
  m_addr.sin_port = htons(m_port);
 
  DebugLog << "create ipv4 address succ [" << toString() << "]";
}

IPAddress::IPAddress(sockaddr_in addr) : m_addr(addr) {
  if (m_addr.sin_family != AF_INET) {
    ErrorLog << "err family, this address is valid";
  }
  m_ip = std::string(inet_ntoa(m_addr.sin_addr));
  m_port = ntohs(m_addr.sin_port);
}

int IPAddress::getFamily() const {
  return m_addr.sin_family;
}

sockaddr* IPAddress::getSockAddr() {
  return reinterpret_cast<sockaddr*>(&m_addr);
}

std::string IPAddress::toString() {
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

std::string UnixDomainAddress::toString() {
  return m_path;
}

}
