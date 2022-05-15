#include <mysql/mysql.h>
#include "tinyrpc/comm/mysql_instase.h"
#include "tinyrpc/comm/config.h"
#include "tinyrpc/comm/log.h"

extern tinyrpc::Config::ptr gRpcConfig;

namespace tinyrpc {

static thread_local MySQLInstaseFactroy* t_mysql_factory = NULL;

MySQLInstaseFactroy* MySQLInstaseFactroy::GetThreadMySQLFactory() {
  if (!t_mysql_factory) {
    return t_mysql_factory;
  }
  t_mysql_factory = new MySQLInstaseFactroy();
  return t_mysql_factory;

}

MySQLInstase* MySQLInstaseFactroy::GetMySQLInstase(const std::string& key) {
  auto it = m_conn_pools.find(key);
  if (it != m_conn_pools.end()) {
    return (it->second).get();
  }
   
  auto it2 = gRpcConfig->m_mysql_options.find(key);
  if (it2 == gRpcConfig->m_mysql_options.end()) {
    ErrorLog << "get MySQLInstase error, not this key[" << key << "] exist";
  }
  DebugLog << "create MySQLInstase of key " << key;
  MySQLInstase::ptr instase = std::make_shared<MySQLInstase>(it2->second);
  m_conn_pools.insert(std::make_pair(key, instase));
  return instase.get();

}


MySQLInstase::MySQLInstase(const MySQLOption& option) {
  Mutex::Lock lock(m_mutex);
  MYSQL* re =  mysql_init(&m_sql_handler);
  lock.unlock();
  if (!re) {
    ErrorLog << "faild to call mysql_init allocate MYSQL instase";
    return;
  }
  int value = 1;
  mysql_options(&m_sql_handler, MYSQL_OPT_RECONNECT, &value);
  if (!option.m_char_set.empty()) {
    mysql_options(&m_sql_handler, MYSQL_SET_CHARSET_NAME, &(option.m_char_set));
  }
  if (!mysql_real_connect(&m_sql_handler, option.m_addr.getIP().c_str(), option.m_user.c_str(), 
      option.m_passwd.c_str(), option.m_select_db.c_str(), option.m_addr.getPort(), NULL, 0)) {
    ErrorLog << "faild to call mysql_real_connect, peer addr[ " << option.m_addr.toString() << "], mysql sys errinfo[" << mysql_error(&m_sql_handler) << "]";
    return;
  }

  m_init_succ = true;

}

bool MySQLInstase::isInitSuccess() {
  return m_init_succ;
}

MySQLInstase::~MySQLInstase() {
  mysql_close(&m_sql_handler);
}

int MySQLInstase::Commit() {
  if (!m_init_succ) {
    ErrorLog << "query error, mysql_handler init faild";
    return -1;
  }
  int rt = mysql_commit(&m_sql_handler);
  if (rt != 0) {
    ErrorLog << "excute mysql_commit error, mysql sys errinfo[" << mysql_error(&m_sql_handler) << "]"; 
  } else {
    InfoLog << "commit success";
  }
  return rt;
}

int MySQLInstase::Query(const std::string& sql) {
  if (!m_init_succ) {
    ErrorLog << "query error, mysql_handler init faild";
    return -1;
  }
  int rt = mysql_real_query(&m_sql_handler, sql.c_str(), sql.length());
  if (rt != 0) {
    ErrorLog << "excute mysql_real_query error, sql[" << sql << "], mysql sys errinfo[" << mysql_error(&m_sql_handler) << "]"; 
  } else {
    InfoLog << "excute mysql_real_query success, sql[" << sql << "]";
  }
  return rt;
}

MYSQL_RES* MySQLInstase::StoreResult() {
  if (!m_init_succ) {
    ErrorLog << "query error, mysql_handler init faild";
    return NULL;
  }
  int count = mysql_field_count(&m_sql_handler);
  if (count != 0) {
    MYSQL_RES* res = mysql_store_result(&m_sql_handler);
    if (!res) {
      ErrorLog << "excute mysql_store_result error, mysql sys errinfo[" << mysql_error(&m_sql_handler) << "]";
    } else {
      DebugLog << "excute mysql_store_result success";
    }
    return res;
  } else {
    DebugLog << "mysql_field_count = 0, not need store result";
    return NULL;
  }

}

MYSQL_ROW MySQLInstase::FetchRow(MYSQL_RES* res) {
  if (!m_init_succ) {
    ErrorLog << "query error, mysql_handler init faild";
    return NULL;
  }
  return mysql_fetch_row(res);
}

long long MySQLInstase::NumFields(MYSQL_RES* res) {
  if (!m_init_succ) {
    ErrorLog << "query error, mysql_handler init faild";
    return -1;
  }
  return mysql_num_fields(res);
}

void MySQLInstase::FreeResult(MYSQL_RES* res) {
  if (!m_init_succ) {
    ErrorLog << "query error, mysql_handler init faild";
    return;
  }
  if (!res) {
    DebugLog << "free result error, res is null";
    return;
  }
  mysql_free_result(res);

}


long long MySQLInstase::AffectedRows() {
  if (!m_init_succ) {
    ErrorLog << "query error, mysql_handler init faild";
    return -1;
  }
  return mysql_affected_rows(&m_sql_handler);

}



}