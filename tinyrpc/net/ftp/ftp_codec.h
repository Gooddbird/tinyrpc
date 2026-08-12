#ifndef TINYRPC_NET_FTP_FTP_CODEC_H
#define TINYRPC_NET_FTP_FTP_CODEC_H

#include <string>
#include "tinyrpc/net/abstract_data.h"
#include "tinyrpc/net/abstract_codec.h"
#include "tinyrpc/net/ftp/ftp_define.h"

namespace tinyrpc {

class FtpCodeC : public AbstractCodeC {
 public:
  FtpCodeC();

  ~FtpCodeC();

  void encode(TcpBuffer* buf, AbstractData* data);

  void decode(TcpBuffer* buf, AbstractData* data);

  ProtocalType getProtocalType();

 private:
  bool parseFtpCommand(FtpCommand* cmd, const std::string& line);
};

}  // namespace tinyrpc

#endif  // TINYRPC_NET_FTP_FTP_CODEC_H
