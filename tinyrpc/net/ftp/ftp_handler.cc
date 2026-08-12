#include "ftp_handler.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <cstdio>
#include <fcntl.h>
#include "tinyrpc/coroutine/coroutine_hook.h"
#include "tinyrpc/comm/log.h"

extern read_fun_ptr_t g_sys_read_fun;
extern write_fun_ptr_t g_sys_write_fun;

namespace tinyrpc {

FtpHandler::FtpHandler(const std::string& root_dir) : m_root_dir(root_dir) {
  m_cmd_handlers = {
    {USER, std::bind(&FtpHandler::handleUSER, this, std::placeholders::_2)},
    {PASS, std::bind(&FtpHandler::handlePASS, this, std::placeholders::_2)},
    {QUIT, std::bind(&FtpHandler::handleQUIT, this, std::placeholders::_2)},
    {CWD,  std::bind(&FtpHandler::handleCWD,  this, std::placeholders::_2)},
    {CDUP, std::bind(&FtpHandler::handleCDUP, this, std::placeholders::_2)},
    {PWD,  std::bind(&FtpHandler::handlePWD,  this, std::placeholders::_2)},
    {MKD,  std::bind(&FtpHandler::handleMKD,  this, std::placeholders::_2)},
    {RMD,  std::bind(&FtpHandler::handleRMD,  this, std::placeholders::_2)},
    {PORT, std::bind(&FtpHandler::handlePORT, this, std::placeholders::_2)},
    {PASV, std::bind(&FtpHandler::handlePASV, this, std::placeholders::_2)},
    {TYPE, std::bind(&FtpHandler::handleTYPE, this, std::placeholders::_2)},
    {SYST, std::bind(&FtpHandler::handleSYST, this, std::placeholders::_2)},
    {DELE, std::bind(&FtpHandler::handleDELE, this, std::placeholders::_2)},
    {RNFR, std::bind(&FtpHandler::handleRNFR, this, std::placeholders::_2)},
    {RNTO, std::bind(&FtpHandler::handleRNTO, this, std::placeholders::_2)},
    {NOOP, std::bind(&FtpHandler::handleNOOP, this, std::placeholders::_2)},
    {FEAT, std::bind(&FtpHandler::handleFEAT, this, std::placeholders::_2)},
    {OPTS, std::bind(&FtpHandler::handleOPTS, this, std::placeholders::_2)},
    {SIZE, std::bind(&FtpHandler::handleSIZE, this, std::placeholders::_2)},
  };
}

FtpHandler::~FtpHandler() {
  closePassiveDataConnection();
}

FtpResponse::ptr FtpHandler::handleCommand(FtpCommand* cmd) {
  auto it = m_cmd_handlers.find(cmd->m_type);
  if (it != m_cmd_handlers.end()) {
    return it->second(this, cmd->m_arg);
  }
  return FtpUtils::MakeResponseFmt(502, "Command not implemented: %s", cmd->m_cmd.c_str());
}

// ========== Auth commands ==========

FtpResponse::ptr FtpHandler::handleUSER(const std::string& arg) {
  if (arg.empty()) {
    return FtpUtils::MakeResponse(501, "Syntax error in parameters");
  }
  m_username = arg;
  m_auth_state = WAIT_PASS;
  return FtpUtils::MakeResponseFmt(331, "User %s OK. Password required", arg.c_str());
}

FtpResponse::ptr FtpHandler::handlePASS(const std::string& arg) {
  if (m_auth_state != WAIT_PASS) {
    return FtpUtils::MakeResponse(503, "Login with USER first");
  }
  m_auth_state = LOGGED_IN;
  return FtpUtils::MakeResponseFmt(230, "User %s logged in", m_username.c_str());
}

FtpResponse::ptr FtpHandler::handleQUIT(const std::string&) {
  return FtpUtils::MakeResponse(221, "Goodbye");
}

// ========== Directory commands ==========

std::string FtpHandler::resolvePath(const std::string& relative) {
  std::string base = m_root_dir;
  if (relative.empty() || relative[0] == '/') {
    base += relative;
  } else {
    base += m_current_dir;
    if (m_current_dir.back() != '/') base += '/';
    base += relative;
  }
  char* resolved = realpath(base.c_str(), nullptr);
  if (resolved) {
    std::string result(resolved);
    free(resolved);
    return result;
  }
  return base;
}

FtpResponse::ptr FtpHandler::handleCWD(const std::string& arg) {
  if (!isLoggedIn()) return FtpUtils::MakeResponse(530, "Not logged in");
  if (arg.empty()) return FtpUtils::MakeResponse(501, "Syntax error");

  std::string new_path = resolvePath(arg);
  struct stat st;
  if (stat(new_path.c_str(), &st) != 0 || !S_ISDIR(st.st_mode)) {
    return FtpUtils::MakeResponse(550, "Failed to change directory");
  }
  if (!FtpUtils::IsPathSafe(m_root_dir, new_path)) {
    return FtpUtils::MakeResponse(550, "Permission denied");
  }

  if (new_path.length() >= m_root_dir.length()) {
    m_current_dir = new_path.substr(m_root_dir.length());
  }
  if (m_current_dir.empty()) m_current_dir = "/";
  return FtpUtils::MakeResponseFmt(250, "Directory changed to %s", m_current_dir.c_str());
}

FtpResponse::ptr FtpHandler::handleCDUP(const std::string&) {
  return handleCWD("..");
}

FtpResponse::ptr FtpHandler::handlePWD(const std::string&) {
  if (!isLoggedIn()) return FtpUtils::MakeResponse(530, "Not logged in");
  return FtpUtils::MakeResponseFmt(257, "\"%s\" is the current directory", m_current_dir.c_str());
}

FtpResponse::ptr FtpHandler::handleMKD(const std::string& arg) {
  if (!isLoggedIn()) return FtpUtils::MakeResponse(530, "Not logged in");
  if (arg.empty()) return FtpUtils::MakeResponse(501, "Syntax error");

  std::string new_path = resolvePath(arg);
  if (!FtpUtils::IsPathSafe(m_root_dir, new_path)) {
    return FtpUtils::MakeResponse(550, "Permission denied");
  }
  if (mkdir(new_path.c_str(), 0755) != 0) {
    return FtpUtils::MakeResponse(550, "Failed to create directory");
  }
  return FtpUtils::MakeResponseFmt(257, "\"%s\" created", arg.c_str());
}

FtpResponse::ptr FtpHandler::handleRMD(const std::string& arg) {
  if (!isLoggedIn()) return FtpUtils::MakeResponse(530, "Not logged in");
  if (arg.empty()) return FtpUtils::MakeResponse(501, "Syntax error");

  std::string path = resolvePath(arg);
  if (!FtpUtils::IsPathSafe(m_root_dir, path)) {
    return FtpUtils::MakeResponse(550, "Permission denied");
  }
  if (rmdir(path.c_str()) != 0) {
    return FtpUtils::MakeResponse(550, "Failed to remove directory");
  }
  return FtpUtils::MakeResponse(250, "Directory removed");
}

// ========== Transfer mode commands ==========

FtpResponse::ptr FtpHandler::handlePORT(const std::string& arg) {
  if (!isLoggedIn()) return FtpUtils::MakeResponse(530, "Not logged in");

  int h1, h2, h3, h4, p1, p2;
  if (sscanf(arg.c_str(), "%d,%d,%d,%d,%d,%d", &h1, &h2, &h3, &h4, &p1, &p2) != 6) {
    return FtpUtils::MakeResponse(501, "Syntax error in PORT");
  }
  char ipbuf[32];
  snprintf(ipbuf, sizeof(ipbuf), "%d.%d.%d.%d", h1, h2, h3, h4);
  m_port_ip = ipbuf;
  m_port_port = p1 * 256 + p2;
  m_data_mode = MODE_ACTIVE;
  return FtpUtils::MakeResponse(200, "PORT command successful");
}

FtpResponse::ptr FtpHandler::handlePASV(const std::string&) {
  if (!isLoggedIn()) return FtpUtils::MakeResponse(530, "Not logged in");

  closePassiveDataConnection();

  m_passive_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (m_passive_listen_fd < 0) {
    return FtpUtils::MakeResponse(425, "Cannot open data connection");
  }

  int opt = 1;
  setsockopt(m_passive_listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = 0;

  if (bind(m_passive_listen_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    close(m_passive_listen_fd);
    m_passive_listen_fd = -1;
    return FtpUtils::MakeResponse(425, "Cannot open data connection");
  }
  if (listen(m_passive_listen_fd, 1) < 0) {
    close(m_passive_listen_fd);
    m_passive_listen_fd = -1;
    return FtpUtils::MakeResponse(425, "Cannot open data connection");
  }

  socklen_t len = sizeof(addr);
  getsockname(m_passive_listen_fd, (struct sockaddr*)&addr, &len);
  m_passive_port = ntohs(addr.sin_port);

  m_data_mode = MODE_PASSIVE;

  int p1 = m_passive_port / 256;
  int p2 = m_passive_port % 256;
  return FtpUtils::MakeResponseFmt(227, "Entering Passive Mode (127,0,0,1,%d,%d)", p1, p2);
}

FtpResponse::ptr FtpHandler::handleTYPE(const std::string& arg) {
  if (!isLoggedIn()) return FtpUtils::MakeResponse(530, "Not logged in");
  if (arg.empty()) return FtpUtils::MakeResponse(501, "Syntax error");

  if (arg == "A" || arg == "A N") {
    m_is_binary = false;
    return FtpUtils::MakeResponse(200, "Type set to A");
  } else if (arg == "I" || arg == "L 8") {
    m_is_binary = true;
    return FtpUtils::MakeResponse(200, "Type set to I");
  }
  return FtpUtils::MakeResponse(504, "Type not implemented for that parameter");
}

FtpResponse::ptr FtpHandler::handleSYST(const std::string&) {
  return FtpUtils::MakeResponse(215, "UNIX Type: L8");
}

// ========== Misc commands ==========

FtpResponse::ptr FtpHandler::handleNOOP(const std::string&) {
  return FtpUtils::MakeResponse(200, "NOOP ok");
}

FtpResponse::ptr FtpHandler::handleFEAT(const std::string&) {
  std::vector<std::string> features = {
    "Features:",
    "SIZE",
    "MDTM",
    "PASV",
    "UTF8",
  };
  auto resp = FtpUtils::MakeMultiResponse(211, features);
  resp->m_lines.push_back("End");
  return resp;
}

FtpResponse::ptr FtpHandler::handleOPTS(const std::string& arg) {
  if (arg == "UTF8 ON") {
    return FtpUtils::MakeResponse(200, "UTF8 mode enabled");
  }
  return FtpUtils::MakeResponse(501, "Option not understood");
}

FtpResponse::ptr FtpHandler::handleDELE(const std::string& arg) {
  if (!isLoggedIn()) return FtpUtils::MakeResponse(530, "Not logged in");
  if (arg.empty()) return FtpUtils::MakeResponse(501, "Syntax error");

  std::string path = resolvePath(arg);
  if (!FtpUtils::IsPathSafe(m_root_dir, path)) {
    return FtpUtils::MakeResponse(550, "Permission denied");
  }
  if (unlink(path.c_str()) != 0) {
    return FtpUtils::MakeResponse(550, "Failed to delete file");
  }
  return FtpUtils::MakeResponse(250, "File deleted");
}

FtpResponse::ptr FtpHandler::handleSIZE(const std::string& arg) {
  if (!isLoggedIn()) return FtpUtils::MakeResponse(530, "Not logged in");
  if (arg.empty()) return FtpUtils::MakeResponse(501, "Syntax error");

  std::string path = resolvePath(arg);
  struct stat st;
  if (stat(path.c_str(), &st) != 0 || !S_ISREG(st.st_mode)) {
    return FtpUtils::MakeResponse(550, "Could not get file size");
  }
  return FtpUtils::MakeResponseFmt(213, "%ld", (long)st.st_size);
}

FtpResponse::ptr FtpHandler::handleRNFR(const std::string& arg) {
  if (!isLoggedIn()) return FtpUtils::MakeResponse(530, "Not logged in");
  if (arg.empty()) return FtpUtils::MakeResponse(501, "Syntax error");

  std::string path = resolvePath(arg);
  struct stat st;
  if (stat(path.c_str(), &st) != 0) {
    return FtpUtils::MakeResponse(550, "File not found");
  }
  m_rename_from = path;
  return FtpUtils::MakeResponse(350, "Ready for RNTO");
}

FtpResponse::ptr FtpHandler::handleRNTO(const std::string& arg) {
  if (!isLoggedIn()) return FtpUtils::MakeResponse(530, "Not logged in");
  if (m_rename_from.empty()) return FtpUtils::MakeResponse(503, "RNFR required first");
  if (arg.empty()) return FtpUtils::MakeResponse(501, "Syntax error");

  std::string path = resolvePath(arg);
  if (!FtpUtils::IsPathSafe(m_root_dir, path)) {
    return FtpUtils::MakeResponse(550, "Permission denied");
  }
  if (rename(m_rename_from.c_str(), path.c_str()) != 0) {
    return FtpUtils::MakeResponse(550, "Rename failed");
  }
  m_rename_from.clear();
  return FtpUtils::MakeResponse(250, "Rename successful");
}

// ========== Data connection helpers ==========

void FtpHandler::closePassiveDataConnection() {
  if (m_passive_listen_fd >= 0) {
    close(m_passive_listen_fd);
    m_passive_listen_fd = -1;
  }
}

// ========== File transfer commands ==========

FtpResponse::ptr FtpHandler::handleLIST(const std::string& arg, int data_fd) {
  std::string target = arg.empty() ? m_current_dir : arg;
  std::string path = resolvePath(target);

  DIR* dir = opendir(path.c_str());
  if (!dir) {
    return FtpUtils::MakeResponse(550, "Failed to open directory");
  }

  std::string listing;
  struct dirent* entry;
  while ((entry = readdir(dir)) != nullptr) {
    std::string name = entry->d_name;
    if (name == "." || name == "..") continue;
    std::string full = path + "/" + name;
    std::string info = FtpUtils::GetFileInfo(full, name);
    if (!info.empty()) {
      listing += info + "\r\n";
    }
  }
  closedir(dir);

  if (data_fd >= 0) {
    write_hook(data_fd, listing.c_str(), listing.length());
  }
  return nullptr;
}

FtpResponse::ptr FtpHandler::handleRETR(const std::string& arg, int data_fd) {
  if (!isLoggedIn()) return FtpUtils::MakeResponse(530, "Not logged in");
  if (data_fd < 0) return FtpUtils::MakeResponse(425, "No data connection");

  std::string path = resolvePath(arg);
  if (!FtpUtils::IsPathSafe(m_root_dir, path)) {
    return FtpUtils::MakeResponse(550, "Permission denied");
  }

  int fd = open(path.c_str(), O_RDONLY);
  if (fd < 0) {
    return FtpUtils::MakeResponse(550, "Failed to open file");
  }

  char buf[65536];
  ssize_t n;
  // SetHook(false);  // disable hook so file read() goes directly to kernel
  while ((n = g_sys_read_fun(fd, buf, sizeof(buf))) > 0) {
    // SetHook(true)
    InfoLog<<"read "<<n<<" bytes";
    ssize_t written = write_hook(data_fd, buf, n);
    InfoLog<<"write "<<written<<" bytes";
    // SetHook(false);
    if (written < 0) break;
  }
  // SetHook(true);
  close(fd);
  return nullptr;
}

FtpResponse::ptr FtpHandler::handleSTOR(const std::string& arg, int data_fd) {
  if (!isLoggedIn()) return FtpUtils::MakeResponse(530, "Not logged in");
  if (data_fd < 0) return FtpUtils::MakeResponse(425, "No data connection");

  std::string path = resolvePath(arg);
  if (!FtpUtils::IsPathSafe(m_root_dir, path)) {
    return FtpUtils::MakeResponse(550, "Permission denied");
  }

  int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd < 0) {
    return FtpUtils::MakeResponse(550, "Failed to create file");
  }

  char buf[65536];
  ssize_t n;
  while ((n = read_hook(data_fd, buf, sizeof(buf))) > 0) {
    SetHook(false);
    g_sys_write_fun(fd, buf, n);
    SetHook(true);
  }
  close(fd);
  return nullptr;
}

}  // namespace tinyrpc
