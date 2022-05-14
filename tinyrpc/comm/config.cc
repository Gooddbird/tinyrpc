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
    printf("exit! read conf file [%s] error info: [%s], errorid: [%d], error_row_column:[%d row %d column]\n", 
      file_path, m_xml_file->ErrorDesc(), m_xml_file->ErrorId(), m_xml_file->ErrorRow(), m_xml_file->ErrorCol());
    exit(0);
  }
}


void Config::readConf() {
  TiXmlElement* root = m_xml_file->RootElement();
  m_log_path = std::string(root->FirstChildElement("log")->FirstChildElement("log_path")->GetText());
  int log_max_size = std::atoi(root->FirstChildElement("log")->FirstChildElement("log_max_file_size")->GetText());
  m_log_max_size = log_max_size * 1024 * 1024;
  m_log_level = stringToLevel(std::string(root->FirstChildElement("log")->FirstChildElement("log_level")->GetText()));

  int cor_stack_size = std::atoi(root->FirstChildElement("coroutine")->FirstChildElement("coroutine_stack_size")->GetText());
  m_cor_stack_size = 1024 * cor_stack_size;

  m_cor_pool_size = std::atoi(root->FirstChildElement("coroutine")->FirstChildElement("coroutine_pool_size")->GetText());

  m_msg_req_len = std::atoi(root->FirstChildElement("msg_req_len")->GetText());
  int max_connect_timeout = std::atoi(root->FirstChildElement("max_connect_timeout")->GetText());
  m_max_connect_timeout = max_connect_timeout * 1000;

  m_iothread_num = std::atoi(root->FirstChildElement("iothread_num")->GetText());
  m_timewheel_bucket_num = std::atoi(root->FirstChildElement("time_wheel")->FirstChildElement("bucket_num")->GetText());
  m_timewheel_inteval = std::atoi(root->FirstChildElement("time_wheel")->FirstChildElement("inteval")->GetText());

  printf("read config from file [%s]: [log_path: %s], [log_max_size: %d MB], [coroutine_stack_size: %d KB], [coroutine_pool_size: %d], [msg_req_len: %d], [max_connect_timeout: %d s], [iothread_num:%d], [timewheel_bucket_num: %d], [timewheel_inteval: %d s]\n",
    m_file_path.c_str(), m_log_path.c_str(), log_max_size, cor_stack_size, m_cor_pool_size, m_msg_req_len, 
    max_connect_timeout, m_iothread_num, m_timewheel_bucket_num, m_timewheel_inteval);
}

Config::~Config() {

}


}