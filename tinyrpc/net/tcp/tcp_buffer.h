#ifndef TINYRPC_NET_TCP_TCP_BUFFER_H
#define TINYRPC_NET_TCP_TCP_BUFFER_H

#include <vector>
#include <memory>


namespace tinyrpc {


class TcpBuffer {
 
 public:
  typedef std::shared_ptr<TcpBuffer> ptr;

  explicit TcpBuffer(int size);
  
  ~TcpBuffer();

  int readAble();

  int writeAble();

  int readIndex() const;

  int writeIndex() const;

  // int readFormSocket(char* buf, int size);

  void writeToBuffer(const char* buf, int size);

  void readFromBuffer(std::vector<char>& re, int size);

  void resizeBuffer(int size);

  void clearBuffer();

  int getSize();

  // const char* getBuffer();

  std::vector<char> getBufferVector();

  std::string getBufferString();

  void recycleRead(int index);

  void recycleWrite(int index);

  void adjustBuffer();

 private:

  int m_read_index {0};
  int m_write_index {0};
  int m_size {0};

 public:
  std::vector<char> m_buffer;

};



}



#endif
