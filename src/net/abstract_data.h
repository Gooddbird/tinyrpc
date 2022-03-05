#ifndef TINYRPC_NET_ABSTRACT_DATA_H
#define TINYRPC_NET_ABSTRACT_DATA_H

namespace tinyrpc {

enum CodeCType {
  CODEC_HTTP = 1,
  CODEC_TINYPB = 2,
};

class AbstractData {
 public:
  AbstractData() {}
  virtual ~AbstractData() {};
  virtual CodeCType type() const = 0;
};


}

#endif