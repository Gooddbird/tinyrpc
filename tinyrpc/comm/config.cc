#include <assert.h>
#include <stdio.h>
#include <memory>
#include <algorithm>
#include "tinyrpc/comm/config.h"
#include "tinyrpc/comm/log.h"
#include "tinyrpc/net/tcp/tcp_server.h"
#include "tinyrpc/net/net_address.h"
#include <tinyxml/tinyxml.h>



namespace tinyrpc {

extern tinyrpc::Logger::ptr gRpcLogger;
extern tinyrpc::TcpServer::ptr gRpcServer;

Config::Config(const char* file_path) : m_file_path(std::string(file_path)) {
  m_xml_file = new TiXmlDocument();
  bool rt = m_xml_file->LoadFile(file_path);
  if (!rt) {
    printf("start tinyrpc server error! read conf file [%s] error info: [%s], errorid: [%d], error_row_column:[%d row %d column]\n", 
      file_path, m_xml_file->ErrorDesc(), m_xml_file->ErrorId(), m_xml_file->ErrorRow(), m_xml_file->ErrorCol());
    exit(0);
  }
}


void Config::readLogConfig(TiXmlElement* log_node) {
  TiXmlElement* node = log_node->FirstChildElement("log_path");
  if(!node || !node->GetText()) {
    printf("start tinyrpc server error! read config file [%s] error, cannot read [log_path] xml node\n", m_file_path.c_str());
    exit(0);
  }
  m_log_path = std::string(node->GetText());

  node = log_node->FirstChildElement("log_prefix");
  if(!node || !node->GetText()) {
    printf("start tinyrpc server error! read config file [%s] error, cannot read [log_prefix] xml node\n", m_file_path.c_str());
    exit(0);
  }
  m_log_prefix = std::string(node->GetText());

  node = log_node->FirstChildElement("log_max_file_size");
  if(!node || !node->GetText()) {
    printf("start tinyrpc server error! read config file [%s] error, cannot read [log_max_file_size] xml node\n", m_file_path.c_str());
    exit(0);
  }

  int log_max_size = std::atoi(node->GetText());
  m_log_max_size = log_max_size * 1024 * 1024;


  node = log_node->FirstChildElement("rpc_log_level");
  if(!node || !node->GetText()) {
    printf("start tinyrpc server error! read config file [%s] error, cannot read [rpc_log_level] xml node\n", m_file_path.c_str());
    exit(0);
  }

  std::string log_level = std::string(node->GetText());
  m_log_level = stringToLevel(log_level);

  node = log_node->FirstChildElement("app_log_level");
  if(!node || !node->GetText()) {
    printf("start tinyrpc server error! read config file [%s] error, cannot read [app_log_level] xml node\n", m_file_path.c_str());
    exit(0);
  }

  log_level = std::string(node->GetText());
  m_app_log_level = stringToLevel(log_level);

  node = log_node->FirstChildElement("log_sync_inteval");
  if(!node || !node->GetText()) {
    printf("start tinyrpc server error! read config file [%s] error, cannot read [log_sync_inteval] xml node\n", m_file_path.c_str());
    exit(0);
  }

  m_log_sync_inteval = std::atoi(node->GetText());

  gRpcLogger = std::make_shared<Logger>();
  gRpcLogger->init(m_log_prefix.c_str(), m_log_path.c_str(), m_log_max_size, m_log_sync_inteval);

}

void Config::readDBConfig(TiXmlElement* node) {
  #ifdef DECLARE_MYSQL_PLUGIN

  printf("read db config\n");
  if (!node) {
    printf("start tinyrpc server error! read config file [%s] error, cannot read [database] xml node\n", m_file_path.c_str());
    exit(0);
  }
  for (TiXmlElement* element = node->FirstChildElement("db_key"); element != NULL; element = element->NextSiblingElement())  {
    std::string key = element->FirstAttribute()->Value();
    printf("key is %s\n", key.c_str());
    TiXmlElement* ip_e = element->FirstChildElement("ip");
    std::string ip;
    int port = 3306;
    if (ip_e) {
      ip = std::string(ip_e->GetText());
    }
    if (ip.empty()) {
      continue;
    }

    TiXmlElement* port_e = element->FirstChildElement("port");
    if (port_e && port_e->GetText()) {
      port = std::atoi(port_e->GetText());
    }
    
    MySQLOption option(IPAddress(ip, port));

    TiXmlElement *user_e = element->FirstChildElement("user");
    if (user_e && user_e->GetText()) {
      option.m_user = std::string(user_e->GetText());
    }

    TiXmlElement *passwd_e = element->FirstChildElement("passwd");
    if (passwd_e && passwd_e->GetText()) {
      option.m_passwd = std::string(passwd_e->GetText());
    }

    TiXmlElement *select_db_e = element->FirstChildElement("select_db");
    if (select_db_e && select_db_e->GetText()) {
      option.m_select_db = std::string(select_db_e->GetText());
    }

    TiXmlElement *char_set_e = element->FirstChildElement("char_set");
    if (char_set_e && char_set_e->GetText()) {
      option.m_char_set = std::string(char_set_e->GetText());
    }
    m_mysql_options.insert(std::make_pair(key, option));
    char buf[512];
    sprintf(buf, "read config from file [%s], key:%s {addr: %s, user: %s, passwd: %s, select_db: %s, charset: %s}\n",
      m_file_path.c_str(), key.c_str(), option.m_addr.toString().c_str(), option.m_user.c_str(),
      option.m_passwd.c_str(), option.m_select_db.c_str(), option.m_char_set.c_str());
    std::string s(buf); 
    InfoLog << s;

  }

  #endif

}


void Config::readConf() {
  TiXmlElement* root = m_xml_file->RootElement();
  TiXmlElement* log_node = root->FirstChildElement("log");
  if (!log_node) {
    printf("start tinyrpc server error! read config file [%s] error, cannot read [log] xml node\n", m_file_path.c_str());
    exit(0);
  }

  readLogConfig(log_node);

  TiXmlElement* time_wheel_node = root->FirstChildElement("time_wheel");
  if (!time_wheel_node) {
    printf("start tinyrpc server error! read config file [%s] error, cannot read [time_wheel] xml node\n", m_file_path.c_str());
    exit(0);
  }

  TiXmlElement* coroutine_node = root->FirstChildElement("coroutine");
  if (!coroutine_node) {
    printf("start tinyrpc server error! read config file [%s] error, cannot read [coroutine] xml node\n", m_file_path.c_str());
    exit(0);
  }

  if (!coroutine_node->FirstChildElement("coroutine_stack_size") || !coroutine_node->FirstChildElement("coroutine_stack_size")->GetText()) {
    printf("start tinyrpc server error! read config file [%s] error, cannot read [coroutine.coroutine_stack_size] xml node\n", m_file_path.c_str());
    exit(0);
  }

  if (!coroutine_node->FirstChildElement("coroutine_pool_size") || !coroutine_node->FirstChildElement("coroutine_pool_size")->GetText()) {
    printf("start tinyrpc server error! read config file [%s] error, cannot read [coroutine.coroutine_pool_size] xml node\n", m_file_path.c_str());
    exit(0);
  }

  int cor_stack_size = std::atoi(coroutine_node->FirstChildElement("coroutine_stack_size")->GetText());
  m_cor_stack_size = 1024 * cor_stack_size;
  m_cor_pool_size = std::atoi(coroutine_node->FirstChildElement("coroutine_pool_size")->GetText());

  if (!root->FirstChildElement("msg_req_len") || !root->FirstChildElement("msg_req_len")->GetText()) {
    printf("start tinyrpc server error! read config file [%s] error, cannot read [msg_req_len] xml node\n", m_file_path.c_str());
    exit(0);
  }

  m_msg_req_len = std::atoi(root->FirstChildElement("msg_req_len")->GetText());

  if (!root->FirstChildElement("max_connect_timeout") || !root->FirstChildElement("max_connect_timeout")->GetText()) {
    printf("start tinyrpc server error! read config file [%s] error, cannot read [max_connect_timeout] xml node\n", m_file_path.c_str());
    exit(0);
  }
  int max_connect_timeout = std::atoi(root->FirstChildElement("max_connect_timeout")->GetText());
  m_max_connect_timeout = max_connect_timeout * 1000;


  if (!root->FirstChildElement("iothread_num") || !root->FirstChildElement("iothread_num")->GetText()) {
    printf("start tinyrpc server error! read config file [%s] error, cannot read [iothread_num] xml node\n", m_file_path.c_str());
    exit(0);
  }

  m_iothread_num = std::atoi(root->FirstChildElement("iothread_num")->GetText());


  if (!time_wheel_node->FirstChildElement("bucket_num") || !time_wheel_node->FirstChildElement("bucket_num")->GetText()) {
    printf("start tinyrpc server error! read config file [%s] error, cannot read [time_wheel.bucket_num] xml node\n", m_file_path.c_str());
    exit(0);
  }
  if (!time_wheel_node->FirstChildElement("inteval") || !time_wheel_node->FirstChildElement("inteval")->GetText()) {
    printf("start tinyrpc server error! read config file [%s] error, cannot read [time_wheel.bucket_num] xml node\n", m_file_path.c_str());
    exit(0);
  }
  m_timewheel_bucket_num = std::atoi(time_wheel_node->FirstChildElement("bucket_num")->GetText());
  m_timewheel_inteval = std::atoi(time_wheel_node->FirstChildElement("inteval")->GetText());

  TiXmlElement* net_node = root->FirstChildElement("server");
  if (!net_node) {
    printf("start tinyrpc server error! read config file [%s] error, cannot read [server] xml node\n", m_file_path.c_str());
    exit(0);
  }

  if (!net_node->FirstChildElement("ip") || !net_node->FirstChildElement("port") || !net_node->FirstChildElement("protocal")) {
    printf("start tinyrpc server error! read config file [%s] error, cannot read [server.ip] or [server.port] or [server.protocal] xml node\n", m_file_path.c_str());
    exit(0);
  }
  std::string ip = std::string(net_node->FirstChildElement("ip")->GetText());
  if (ip.empty()) {
    ip = "0.0.0.0";
  }
  int port = std::atoi(net_node->FirstChildElement("port")->GetText());
  if (port == 0) {
    printf("start tinyrpc server error! read config file [%s] error, read [server.port] = 0\n", m_file_path.c_str());
    exit(0);
  }
  std::string protocal = std::string(net_node->FirstChildElement("protocal")->GetText());
  std::transform(protocal.begin(), protocal.end(), protocal.begin(), toupper);

  tinyrpc::IPAddress::ptr addr = std::make_shared<tinyrpc::IPAddress>(ip, port);

  if (protocal == "HTTP") {
    gRpcServer = std::make_shared<TcpServer>(addr, Http_Protocal);
  } else {
    gRpcServer = std::make_shared<TcpServer>(addr, TinyPb_Protocal);
  }

  char buff[512];
  sprintf(buff, "read config from file [%s]: [log_path: %s], [log_prefix: %s], [log_max_size: %d MB], [log_level: %s], " 
      "[coroutine_stack_size: %d KB], [coroutine_pool_size: %d], "
      "[msg_req_len: %d], [max_connect_timeout: %d s], "
      "[iothread_num:%d], [timewheel_bucket_num: %d], [timewheel_inteval: %d s], [server_ip: %s], [server_Port: %d], [server_protocal: %s]\n",
      m_file_path.c_str(), m_log_path.c_str(), m_log_prefix.c_str(), m_log_max_size / 1024 / 1024, 
      levelToString(m_log_level).c_str(), cor_stack_size, m_cor_pool_size, m_msg_req_len,
      max_connect_timeout, m_iothread_num, m_timewheel_bucket_num, m_timewheel_inteval, ip.c_str(), port, protocal.c_str());

  std::string s(buff);
  InfoLog << s;

  TiXmlElement* database_node = root->FirstChildElement("database");

  if (database_node) {
    readDBConfig(database_node);
  }

}

Config::~Config() {
  if (m_xml_file) {
    delete m_xml_file;
    m_xml_file = NULL;
  }
}


TiXmlElement* Config::getXmlNode(const std::string& name) {
  return m_xml_file->RootElement()->FirstChildElement(name.c_str());
}

}
