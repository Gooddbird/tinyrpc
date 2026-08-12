#ifndef TINYRPC_NET_FTP_FTP_DEFINE_H
#define TINYRPC_NET_FTP_FTP_DEFINE_H

#include <string>
#include <vector>
#include <memory>
#include <map>
#include "tinyrpc/net/abstract_data.h"

namespace tinyrpc {

enum FtpCommandType {
  USER = 1,
  PASS,
  ACCT,
  CWD,
  CDUP,
  SMNT,
  QUIT,
  REIN,
  PORT,
  PASV,
  TYPE,
  STRU,
  MODE,
  RETR,
  STOR,
  STOU,
  APPE,
  ALLO,
  REST,
  RNFR,
  RNTO,
  ABOR,
  DELE,
  RMD,
  MKD,
  PWD,
  LIST,
  NLST,
  SITE,
  SYST,
  STAT,
  HELP,
  NOOP,
  SIZE,
  MDTM,
  FEAT,
  OPTS,
  UNKNOWN,
};

const std::map<std::string, FtpCommandType> g_ftp_cmd_map = {
  {"USER", USER}, {"PASS", PASS}, {"ACCT", ACCT},
  {"CWD",  CWD},  {"CDUP", CDUP}, {"SMNT", SMNT},
  {"QUIT", QUIT}, {"REIN", REIN},
  {"PORT", PORT}, {"PASV", PASV},
  {"TYPE", TYPE}, {"STRU", STRU}, {"MODE", MODE},
  {"RETR", RETR}, {"STOR", STOR}, {"STOU", STOU},
  {"APPE", APPE}, {"ALLO", ALLO}, {"REST", REST},
  {"RNFR", RNFR}, {"RNTO", RNTO}, {"ABOR", ABOR},
  {"DELE", DELE}, {"RMD",  RMD},  {"MKD",  MKD},
  {"PWD",  PWD},  {"LIST", LIST}, {"NLST", NLST},
  {"SITE", SITE}, {"SYST", SYST}, {"STAT", STAT},
  {"HELP", HELP}, {"NOOP", NOOP},
  {"SIZE", SIZE}, {"MDTM", MDTM},
  {"FEAT", FEAT}, {"OPTS", OPTS},
};

class FtpCommand : public AbstractData {
 public:
  typedef std::shared_ptr<FtpCommand> ptr;

  FtpCommandType m_type {UNKNOWN};
  std::string m_cmd;
  std::string m_arg;
  bool m_welcomed {false};
};

class FtpResponse : public AbstractData {
 public:
  typedef std::shared_ptr<FtpResponse> ptr;

  int m_code {0};
  std::vector<std::string> m_lines;
};

}  // namespace tinyrpc

#endif  // TINYRPC_NET_FTP_FTP_DEFINE_H
