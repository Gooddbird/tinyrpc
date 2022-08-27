#ifndef TINYRPC_NET_TINYPB_TINYPB_RPC_CLOSURE_H
#define TINYRPC_NET_TINYPB_TINYPB_RPC_CLOSURE_H

#include <google/protobuf/stubs/callback.h>
#include <functional>


namespace tinyrpc {

class TinyPbRpcClosure : public google::protobuf::Closure {
 public:
  typedef std::shared_ptr<TinyPbRpcClosure> ptr;
  explicit TinyPbRpcClosure(std::function<void()> cb) : m_cb(cb) {

  }

  ~TinyPbRpcClosure() = default;

  void Run() {
    if(m_cb) {
      m_cb();
    }
  }

 private:
  std::function<void()> m_cb {nullptr};

};

}


#endif