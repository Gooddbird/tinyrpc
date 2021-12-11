#include <unistd.h>
#include "tcp_buffer.h"


namespace tinyrpc {

TcpBuffer::TcpBuffer(size) {
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

int TcpBuffer::readFromSocket(int sockfd) {
	if (writeAble() == 0) {
		m_buffer.resize(2 * m_size);
	}
	int rt = read(sockfd, &m_buffer[m_write_index], writeAble());
	if (rt >= 0) {
		m_write_index += rt;
	}
	return rt;
}

void TcpBuffer::writeToBuffer(const char* buf, int size) {
	if (size > writeAble()) {
		m_buffer.resize(size + m_write_index);		
	}
	std::copy(buf, buf + size, &m_buffer[m_write_index]);
	m_write_index += size;
}

}
