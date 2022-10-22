#include <google/protobuf/service.h>
#include <exception>
#include "charon/server/service/charon.h"
#include "charon/server/interface/discover_server.h"
#include "charon/server/interface/register_server.h"
#include "charon/server/interface/ask_vote.h"
#include "charon/server/interface/append_log_entries.h"
#include "charon/comm/exception.h"
#include "tinyrpc/comm/log.h"
#include "tinyrpc/comm/start.h"
#include "tinyrpc/net/tcp/io_thread.h"
#include "tinyrpc/coroutine/coroutine.h"

#define CALL_CHARON_INTERFACE(type)                                                                                                 \
  type impl(request, response);                                                                                                     \
  response->set_ret_code(0);                                                                                                        \
  response->set_res_info("OK");                                                                                                     \
  try {                                                                                                                             \
    AppInfoLog << "request:{" << request->ShortDebugString() << "}";                                                                \
    impl.run();                                                                                                                     \
    AppInfoLog << "response:{" << response->ShortDebugString() << "}";                                                              \
  } catch (charon::CharonException& e) {                                                                                            \
    AppErrorLog << "occur CharonException, error code = " << e.code() << ", errinfo = " << e.error();                               \
    response->set_ret_code(e.code());                                                                                               \
    response->set_res_info(e.error());                                                                                              \
  } catch (std::exception&) {                                                                                                       \
    AppErrorLog << "occur std::exception, error code = -1, errorinfo = UnKnown error ";                                             \
    response->set_ret_code(-1);                                                                                                     \
    response->set_res_info("UnKnown error");                                                                                        \
  } catch (...) {                                                                                                                   \
    AppErrorLog << "occur UnKnown exception, error code = -1, errorinfo = UnKnown error ";                                          \
    response->set_ret_code(-1);                                                                                                     \
    response->set_res_info("UnKnown error");                                                                                        \
  }                                                                                                                                 \
  if (done) {                                                                                                                       \
    done->Run();                                                                                                                    \
  }                                                                                                                                 \

namespace charon {

void Charon::DiscoverServer(::google::protobuf::RpcController* controller,
                      const ::DiscoverRequest* request,
                      ::DiscoverResponse* response,
                      ::google::protobuf::Closure* done) {

  CALL_CHARON_INTERFACE(DiscoverServerImpl);
}

void Charon::RegisterServer(::google::protobuf::RpcController* controller,
                      const ::RegisterRequest* request,
                      ::RegisterResponse* response,
                      ::google::protobuf::Closure* done) {

  CALL_CHARON_INTERFACE(RegisterServerImpl);
}


void Raft::AskVote(::google::protobuf::RpcController* controller,
                      const ::AskVoteRequest* request,
                      ::AskVoteResponse* response,
                      ::google::protobuf::Closure* done) {


  CALL_CHARON_INTERFACE(AskVoteImpl);
}

void Raft::AppendLogEntries(::google::protobuf::RpcController* controller,
                      const ::AppendLogEntriesRequest* request,
                      ::AppendLogEntriesResponse* response,
                      ::google::protobuf::Closure* done) {

  CALL_CHARON_INTERFACE(AppendLogEntriesImpl);
}

}