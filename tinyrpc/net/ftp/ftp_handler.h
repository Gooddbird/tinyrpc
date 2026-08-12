#ifndef TINYRPC_NET_FTP_FTP_HANDLER_H
#define TINYRPC_NET_FTP_FTP_HANDLER_H

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <map>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include "tinyrpc/net/ftp/ftp_define.h"
#include "tinyrpc/comm/log.h"

namespace tinyrpc {

namespace FtpUtils {

// Build {"code", "text"} single-line response
inline FtpResponse::ptr MakeResponse(int code, const std::string& text) {
  auto resp = std::make_shared<FtpResponse>();
  resp->m_code = code;
  resp->m_lines.push_back(text);
  return resp;
}

// Build single-line response with formatted text
template<typename... Args>
inline FtpResponse::ptr MakeResponseFmt(int code, const char* fmt, Args... args) {
  auto resp = std::make_shared<FtpResponse>();
  resp->m_code = code;
  char buf[512];
  snprintf(buf, sizeof(buf), fmt, args...);
  resp->m_lines.push_back(std::string(buf));
  return resp;
}

// Build multi-line response
inline FtpResponse::ptr MakeMultiResponse(int code, const std::vector<std::string>& lines) {
  auto resp = std::make_shared<FtpResponse>();
  resp->m_code = code;
  resp->m_lines = lines;
  return resp;
}

// Check if path is safe (relative to root)
inline bool IsPathSafe(const std::string& root, const std::string& path) {
  char* resolved = realpath(path.c_str(), nullptr);
  if (!resolved) return false;
  bool safe = (strncmp(resolved, root.c_str(), root.length()) == 0);
  free(resolved);
  return safe;
}

// Get file info string like "-rw-r--r-- 1 owner group 1234 Jan 01 12:00 filename"
inline std::string GetFileInfo(const std::string& filepath, const std::string& filename) {
  struct stat st;
  if (stat(filepath.c_str(), &st) != 0) {
    return "";
  }
  char buf[256];
  // Permission string
  char perm[11] = "----------";
  if (S_ISDIR(st.st_mode))  perm[0] = 'd';
  if (st.st_mode & S_IRUSR) perm[1] = 'r';
  if (st.st_mode & S_IWUSR) perm[2] = 'w';
  if (st.st_mode & S_IXUSR) perm[3] = 'x';
  if (st.st_mode & S_IRGRP) perm[4] = 'r';
  if (st.st_mode & S_IWGRP) perm[5] = 'w';
  if (st.st_mode & S_IXGRP) perm[6] = 'x';
  if (st.st_mode & S_IROTH) perm[7] = 'r';
  if (st.st_mode & S_IWOTH) perm[8] = 'w';
  if (st.st_mode & S_IXOTH) perm[9] = 'x';

  char timebuf[20];
  strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", localtime(&st.st_mtime));
  snprintf(buf, sizeof(buf), "%s %3lu ftp ftp %8ld %s %s",
           perm, (unsigned long)st.st_nlink, (long)st.st_size, timebuf, filename.c_str());
  return std::string(buf);
}

}  // namespace FtpUtils

class FtpDispatcher;  // forward declaration

class FtpHandler {
  friend class FtpDispatcher;

 public:
  typedef std::shared_ptr<FtpHandler> ptr;

  enum AuthState {
    NOT_LOGGED_IN = 0,
    WAIT_PASS,
    LOGGED_IN,
  };

  enum DataMode {
    MODE_NONE = 0,
    MODE_ACTIVE,
    MODE_PASSIVE,
  };

  FtpHandler(const std::string& root_dir);
  ~FtpHandler();

  // Dispatch a command and return the response
  FtpResponse::ptr handleCommand(FtpCommand* cmd);

  // Accessors
  const std::string& getRootDir() const { return m_root_dir; }
  bool isLoggedIn() const { return m_auth_state == LOGGED_IN; }

 private:
  // Auth commands
  FtpResponse::ptr handleUSER(const std::string& arg);
  FtpResponse::ptr handlePASS(const std::string& arg);
  FtpResponse::ptr handleQUIT(const std::string& arg);

  // Directory commands
  FtpResponse::ptr handleCWD(const std::string& arg);
  FtpResponse::ptr handleCDUP(const std::string& arg);
  FtpResponse::ptr handlePWD(const std::string& arg);
  FtpResponse::ptr handleMKD(const std::string& arg);
  FtpResponse::ptr handleRMD(const std::string& arg);

  // Transfer setup commands
  FtpResponse::ptr handlePORT(const std::string& arg);
  FtpResponse::ptr handlePASV(const std::string& arg);
  FtpResponse::ptr handleTYPE(const std::string& arg);
  FtpResponse::ptr handleSYST(const std::string& arg);

  // File transfer commands
  FtpResponse::ptr handleLIST(const std::string& arg, int data_fd);
  FtpResponse::ptr handleRETR(const std::string& arg, int data_fd);
  FtpResponse::ptr handleSTOR(const std::string& arg, int data_fd);
  FtpResponse::ptr handleDELE(const std::string& arg);

  // Rename commands
  FtpResponse::ptr handleRNFR(const std::string& arg);
  FtpResponse::ptr handleRNTO(const std::string& arg);

  // Misc commands
  FtpResponse::ptr handleNOOP(const std::string& arg);
  FtpResponse::ptr handleFEAT(const std::string& arg);
  FtpResponse::ptr handleOPTS(const std::string& arg);
  FtpResponse::ptr handleSIZE(const std::string& arg);

  // Internal helpers
  std::string resolvePath(const std::string& relative);
  void closePassiveDataConnection();

 private:
  AuthState m_auth_state {NOT_LOGGED_IN};
  DataMode m_data_mode {MODE_NONE};

  std::string m_root_dir;
  std::string m_current_dir {"/"};

  std::string m_username;
  std::string m_rename_from;

  // Active mode: client address from PORT command
  std::string m_port_ip;
  int m_port_port {0};

  // Passive mode: server listen socket
  int m_passive_listen_fd {-1};
  int m_passive_port {0};

  bool m_is_binary {false};  // TYPE A or I

  typedef std::function<FtpResponse::ptr(FtpHandler*, const std::string&)> CmdHandler;
  std::map<FtpCommandType, CmdHandler> m_cmd_handlers;
};

}  // namespace tinyrpc

#endif  // TINYRPC_NET_FTP_FTP_HANDLER_H
