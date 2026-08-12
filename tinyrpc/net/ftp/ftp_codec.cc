#include <sstream>
#include <algorithm>
#include <cstring>
#include "ftp_codec.h"
#include "tinyrpc/comm/log.h"
#include "tinyrpc/net/tcp/tcp_buffer.h"

namespace tinyrpc {

static const char* g_CRLF = "\r\n";
static const size_t g_CRLF_LEN = 2;

static const char* g_CRLF_DOUBLE = "\r\n\r\n";

FtpCodeC::FtpCodeC() {}

FtpCodeC::~FtpCodeC() {}

void FtpCodeC::encode(TcpBuffer* buf, AbstractData* data) {
  FtpResponse* response = dynamic_cast<FtpResponse*>(data);
  if (!response) {
    ErrorLog << "ftp encode error: data is not FtpResponse";
    return;
  }
  response->encode_succ = false;

  std::stringstream ss;
  size_t line_count = response->m_lines.size();

  if (line_count == 0) {
    // No lines, encode just the code
    ss << response->m_code << g_CRLF;
  } else if (line_count == 1) {
    // Single-line response: "code text\r\n"
    ss << response->m_code << " " << response->m_lines[0] << g_CRLF;
  } else {
    // Multi-line response:
    //   "code-text\r\n"
    //   ...
    //   "code text\r\n"
    for (size_t i = 0; i < line_count; ++i) {
      if (i == line_count - 1) {
        ss << response->m_code << " " << response->m_lines[i] << g_CRLF;
      } else {
        ss << response->m_code << "-" << response->m_lines[i] << g_CRLF;
      }
    }
  }

  std::string ftp_res = ss.str();
  DebugLog << "ftp encode: " << ftp_res.substr(0, ftp_res.length() - 2);  // strip trailing CRLF

  buf->writeToBuffer(ftp_res.c_str(), ftp_res.length());
  response->encode_succ = true;
}

void FtpCodeC::decode(TcpBuffer* buf, AbstractData* data) {
  DebugLog<<"ftp receive data:"<<buf->getBufferString();
  if (!buf || !data) {
    ErrorLog << "ftp decode error: buf or data nullptr";
    return;
  }

  FtpCommand* cmd = dynamic_cast<FtpCommand*>(data);
  if (!cmd) {
    ErrorLog << "ftp decode error: data is not FtpCommand";
    return;
  }

  cmd->decode_succ = false;

  std::string strs = buf->getBufferString();
  if (strs.empty()) {
    DebugLog << "ftp decode: buffer is empty";
    return;
  }

  // Strip Telnet IAC negotiation sequences before parsing FTP command.
  // Standard FTP clients send IAC WILL/DONT/DO/WONT sequences
  // (e.g. \xff\xfb\x01 for WILL ECHO). Each IAC sequence is 3 bytes.
  // If a bare IAC appears at the end of the buffer (incomplete sequence),
  // skip it and wait for more data.
  std::string clean;
  clean.reserve(strs.size());
  for (size_t pos = 0; pos < strs.size(); ) {
    if (static_cast<unsigned char>(strs[pos]) == 0xff) {
      // IAC byte found — skip this 3-byte Telnet sequence
      if (pos + 2 < strs.size()) {
        pos += 3;
      } else {
        // Incomplete IAC sequence at end, skip it
        pos = strs.size();
      }
    } else {
      clean.push_back(strs[pos]);
      ++pos;
    }
  }

  // Find CRLF delimiter in cleaned content
  size_t i = clean.find(g_CRLF);
  if (i == std::string::npos) {
    DebugLog << "ftp decode: no CRLF found in buffer, waiting for more data";
    return;
  }

  std::string line = clean.substr(0, i);
  if (line.empty()) {
    // Empty line (just CRLF) after telnet stripping, consume original data
    DebugLog << "ftp decode: empty line, skipping";
    buf->recycleRead(strs.find(g_CRLF) + g_CRLF_LEN);
    return;
  }

  DebugLog << "ftp decode parsing line: '" << line << "'";

  if (!parseFtpCommand(cmd, line)) {
    ErrorLog << "ftp decode: failed to parse command line";
    return;
  }

  // Consume all original data up to and including the CRLF
  size_t orig_pos = strs.find(g_CRLF);
  buf->recycleRead(orig_pos + g_CRLF_LEN);
  cmd->decode_succ = true;

  DebugLog << "ftp decode success: cmd=" << cmd->m_cmd << ", arg=" << cmd->m_arg;
}

bool FtpCodeC::parseFtpCommand(FtpCommand* cmd, const std::string& line) {
  // FTP command format: <COMMAND> [<argument>]
  // Command is case-insensitive

  if (line.empty()) {
    return false;
  }

  // Split by first space
  size_t space_pos = line.find(' ');
  std::string cmd_name;
  std::string arg;

  if (space_pos == std::string::npos) {
    cmd_name = line;
    arg.clear();
  } else {
    cmd_name = line.substr(0, space_pos);
    arg = line.substr(space_pos + 1);
  }

  // Convert to uppercase for command lookup
  std::transform(cmd_name.begin(), cmd_name.end(), cmd_name.begin(), ::toupper);

  auto it = g_ftp_cmd_map.find(cmd_name);
  if (it == g_ftp_cmd_map.end()) {
    cmd->m_type = UNKNOWN;
    cmd->m_cmd = cmd_name;
    cmd->m_arg = arg;
    DebugLog << "ftp unknown command: " << cmd_name;
  } else {
    cmd->m_type = it->second;
    cmd->m_cmd = cmd_name;
    cmd->m_arg = arg;
  }

  return true;
}

ProtocalType FtpCodeC::getProtocalType() {
  return Ftp_Protocal;
}

}  // namespace tinyrpc
