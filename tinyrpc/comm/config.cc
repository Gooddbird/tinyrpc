#include <tinyxml/tinyxml.h>
#include <assert.h>
#include <stdio.h>
#include "config.h"
#include "log.h"


namespace tinyrpc {

Config::Config(const char* file_path) : m_file_path(std::string(file_path)) {
  m_xml_file = new TiXmlDocument();
  bool rt = m_xml_file->LoadFile(file_path);
  if (!rt) {
    printf("start tinyrpc server error! read conf file [%s] error info: [%s], errorid: [%d], error_row_column:[%d row %d column]\n", 
      file_path, m_xml_file->ErrorDesc(), m_xml_file->ErrorId(), m_xml_file->ErrorRow(), m_xml_file->ErrorCol());
    exit(0);
  }
}

void Config::readDBConfig(TiXmlElement* node) {
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
    if (port_e) {
      port = std::atoi(port_e->GetText());
    }
    MySQLOption option(IPAddress(ip, port));

    TiXmlElement *user_e = element->FirstChildElement("user");
    if (user_e) {
      option.m_user = std::string(user_e->GetText());
    }

    TiXmlElement *passwd_e = element->FirstChildElement("passwd");
    if (passwd_e) {
      option.m_passwd = std::string(passwd_e->GetText());
    }

    TiXmlElement *select_db_e = element->FirstChildElement("select_db");
    if (select_db_e) {
      option.m_select_db = std::string(select_db_e->GetText());
    }

    TiXmlElement *char_set_e = element->FirstChildElement("char_set");
    if (char_set_e) {
      option.m_char_set = std::string(char_set_e->GetText());
    }
    m_mysql_options.insert(std::make_pair(key, option));
    printf("read config from file [%s], key:%s {addr: %s, user: %s, passwd: %s, select_db: %s, charset: %s}\n",
      m_file_path.c_str(), key.c_str(), option.m_addr.toString().c_str(), option.m_user.c_str(),
      option.m_passwd.c_str(), option.m_select_db.c_str(), option.m_char_set.c_str());
    
  }

}


void Config::readConf() {
  TiXmlElement* root = m_xml_file->RootElement();
  TiXmlElement* log_node = root->FirstChildElement("log");
  if (!log_node) {
    printf("start tinyrpc server error! read config file [%s] error, cannot read [log] xml node", m_file_path.c_str());
    exit(0);
  }

  TiXmlElement* coroutine_node = root->FirstChildElement("coroutine");
  if (!coroutine_node) {
    printf("start tinyrpc server error! read config file [%s] error, cannot read [coroutine] xml node", m_file_path.c_str());
    exit(0);
  }

  TiXmlElement* time_wheel_node = root->FirstChildElement("time_wheel");
  if (!time_wheel_node) {
    printf("start tinyrpc server error! read config file [%s] error, cannot read [time_wheel] xml node", m_file_path.c_str());
    exit(0);
  }

  m_log_path = std::string(log_node->FirstChildElement("log_path")->GetText());
  int log_max_size = std::atoi(log_node->FirstChildElement("log_max_file_size")->GetText());
  m_log_max_size = log_max_size * 1024 * 1024;
  std::string log_level = std::string(log_node->FirstChildElement("log_level")->GetText());
  m_log_level = stringToLevel(log_level);

  int log_sync_inteval = std::atoi(log_node->FirstChildElement("log_sync_inteval")->GetText());
  m_log_sync_inteval = log_sync_inteval * 1000;

  int cor_stack_size = std::atoi(root->FirstChildElement("coroutine")->FirstChildElement("coroutine_stack_size")->GetText());
  m_cor_stack_size = 1024 * cor_stack_size;

  m_cor_pool_size = std::atoi(root->FirstChildElement("coroutine")->FirstChildElement("coroutine_pool_size")->GetText());

  m_msg_req_len = std::atoi(root->FirstChildElement("msg_req_len")->GetText());
  int max_connect_timeout = std::atoi(root->FirstChildElement("max_connect_timeout")->GetText());
  m_max_connect_timeout = max_connect_timeout * 1000;

  m_iothread_num = std::atoi(root->FirstChildElement("iothread_num")->GetText());
  m_timewheel_bucket_num = std::atoi(root->FirstChildElement("time_wheel")->FirstChildElement("bucket_num")->GetText());
  m_timewheel_inteval = std::atoi(root->FirstChildElement("time_wheel")->FirstChildElement("inteval")->GetText());


  printf("read config from file [%s]: [log_path: %s], [log_max_size: %d MB], [log_level: %s], " 
      "[coroutine_stack_size: %d KB], [coroutine_pool_size: %d], "
      "[msg_req_len: %d], [max_connect_timeout: %d s], "
      "[iothread_num:%d], [timewheel_bucket_num: %d], [timewheel_inteval: %d s]\n",
      m_file_path.c_str(), m_log_path.c_str(), log_max_size, levelToString(m_log_level).c_str(), cor_stack_size, m_cor_pool_size, m_msg_req_len,
      max_connect_timeout, m_iothread_num, m_timewheel_bucket_num, m_timewheel_inteval);

  TiXmlElement* database_node = root->FirstChildElement("database");
  if (database_node) {
    readDBConfig(database_node);
  }

}

Config::~Config() {

}


}