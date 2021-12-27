#include <unistd.h>
#include <string.h>
#include "tcp_buffer.h"
#include "../../log/log.h"


namespace tinyrpc {

TcpBuffer::TcpBuffer(int size) {
	m_buffer.resize(size);	
}


TcpBuffer::~TcpBuffer() {

}

int TcpBuffer::readAble() {

	return m_write_index - m_read_index;
}

int TcpBuffer::writeAble() {
	
	return m_buffer.size() - m_write_index;
}

// int TcpBuffer::readFromSocket(int sockfd) {
	// if (writeAble() == 0) {
		// m_buffer.resize(2 * m_size);
	// }
	// int rt = read(sockfd, &m_buffer[m_write_index], writeAble());
	// if (rt >= 0) {
		// m_write_index += rt;
	// }
	// return rt;
// }

void TcpBuffer::resize(int size) {
  std::vector<char> tmp(size);
  int c = std::min(size, readAble());
  memcpy(&tmp[0], &m_buffer[m_read_index], c);

  m_buffer.swap(tmp);
  m_read_index = 0;
  m_write_index = m_read_index + c;

}

void TcpBuffer::writeToBuffer(const char* buf, int size) {
	if (size > writeAble()) {
		m_buffer.resize(size + m_write_index);		
	}
	memcpy(&m_buffer[m_write_index], buf, size);
	m_write_index += size;

}


void TcpBuffer::readFromBuffer(std::vector<char>& re, int size) {
  if (readAble() == 0) {
    DebugLog << "read buffer empty!";
    return; 
  }
  int read_size = readAble() > size ? size : readAble();
  std::vector<char> tmp(read_size); 

  // std::copy(m_read_index, m_read_index + read_size, tmp);
  memcpy(&tmp[0], &m_buffer[m_read_index], read_size);
  re.swap(tmp);
  m_read_index += read_size;

  if (m_read_index > static_cast<int>(m_buffer.size() / 3)) {
    
    std::vector<char> new_buffer(m_buffer.size());

    // std::copy(&m_buffer[m_read_index], readAble(), &new_buffer);
    memcpy(&new_buffer[0], &m_buffer[m_read_index], readAble());

    m_buffer.swap(new_buffer);
    m_read_index = 0;
    m_write_index -= m_read_index;
    
  }

}

int TcpBuffer::getSize() {
  return m_buffer.size();
}

void TcpBuffer::clear() {
  m_buffer.clear();
  m_read_index = 0;
  m_write_index = 0;
}

}
