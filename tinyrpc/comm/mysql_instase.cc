
#ifdef DECLARE_MYSQL_PLUGIN 
#include <mysql/mysql.h>
#include <mysql/errmsg.h>
#endif

#include "tinyrpc/comm/mysql_instase.h"
#include "tinyrpc/comm/config.h"
#include "tinyrpc/comm/log.h"
#include "tinyrpc/coroutine/coroutine_hook.h"

extern tinyrpc::Config::ptr gRpcConfig;

namespace tinyrpc {

#ifdef DECLARE_MYSQL_PLUGIN 

static thread_local MySQLInstaseFactroy* t_mysql_factory = NULL;


MySQLThreadInit::MySQLThreadInit() {
  DebugLog << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< call mysql_thread_init";
  mysql_thread_init();
}

MySQLThreadInit::~MySQLThreadInit() {
  mysql_thread_end();
  DebugLog << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> call mysql_thread_end";
}

MySQLInstaseFactroy* MySQLInstaseFactroy::GetThreadMySQLFactory() {
  if (t_mysql_factory) {
    return t_mysql_factory;
  }
  t_mysql_factory = new MySQLInstaseFactroy();
  return t_mysql_factory;

}

MySQLInstase::ptr MySQLInstaseFactroy::GetMySQLInstase(const std::string& key) {
  auto it2 = gRpcConfig->m_mysql_options.find(key);
  if (it2 == gRpcConfig->m_mysql_options.end()) {
    ErrorLog << "get MySQLInstase error, not this key[" << key << "] exist";
    return NULL;
  }
  DebugLog << "create MySQLInstase of key " << key;
  MySQLInstase::ptr instase = std::make_shared<MySQLInstase>(it2->second);
  return instase;
}


MySQLInstase::MySQLInstase(const MySQLOption& option) : m_option(option) {
  int ret = reconnect();
  if (ret != 0) {
    return;
  }

  m_init_succ = true;

}

int MySQLInstase::reconnect() {
  // this static value only call once
  // it will call mysql_thread_init when first call MySQLInstase::reconnect function
  // and it will call mysql_thread_end when current thread destroy
  static thread_local MySQLThreadInit t_mysql_thread_init;

  if (m_sql_handler) {
    mysql_close(m_sql_handler);
    m_sql_handler = NULL;
  }

  Mutex::Lock lock(m_mutex);
  m_sql_handler =  mysql_init(NULL);
  // DebugLog << "mysql fd is " << m_sql_handler.net.fd;
  lock.unlock();
  if (!m_sql_handler) {
    ErrorLog << "faild to call mysql_init allocate MYSQL instase";
    return -1;
  }
  // int value = 0;
  // mysql_options(m_sql_handler, MYSQL_OPT_RECONNECT, &value);
  if (!m_option.m_char_set.empty()) {
    mysql_options(m_sql_handler, MYSQL_SET_CHARSET_NAME, m_option.m_char_set.c_str());
  }
  DebugLog << "begin to connect mysql{ip:" << m_option.m_addr.getIP() << ", port:" << m_option.m_addr.getPort() 
    << ", user:" << m_option.m_user << ", passwd:" << m_option.m_passwd << ", select_db: "<< m_option.m_select_db << "charset:" << m_option.m_char_set << "}";
  // mysql_real_connect(m_sql_handler, m_option.m_addr.getIP().c_str(), m_option.m_user.c_str(), 
  //     m_option.m_passwd.c_str(), m_option.m_select_db.c_str(), m_option.m_addr.getPort(), NULL, 0);
  if (!mysql_real_connect(m_sql_handler, m_option.m_addr.getIP().c_str(), m_option.m_user.c_str(), 
      m_option.m_passwd.c_str(), m_option.m_select_db.c_str(), m_option.m_addr.getPort(), NULL, 0)) {

    ErrorLog << "faild to call mysql_real_connect, peer addr[ " << m_option.m_addr.getIP() << ":" << m_option.m_addr.getPort() << "], mysql sys errinfo[" << mysql_error(m_sql_handler) << "]";
    return -1;
  }
  DebugLog << "mysql_handler connect succ";
  return 0;
}

bool MySQLInstase::isInitSuccess() {
  return m_init_succ;
}

MySQLInstase::~MySQLInstase() {
  if (m_sql_handler) {
    mysql_close(m_sql_handler);
    m_sql_handler = NULL;
  }
}

int MySQLInstase::commit() {
  int rt = query("COMMIT;");
  if (rt == 0) {
    m_in_trans = false;
  }
  return rt;
}

int MySQLInstase::begin() {
  int rt = query("BEGIN;");
  if (rt == 0) {
    m_in_trans = true;
  }
  return rt;
}

int MySQLInstase::rollBack() {
  int rt = query("ROLLBACK;");
  if (rt == 0) {
    m_in_trans = false;
  }
  return rt;
}

int MySQLInstase::query(const std::string& sql) {
  if (!m_init_succ) {
    ErrorLog << "query error, mysql_handler init faild";
    return -1;
  }
  if (!m_sql_handler) {
    DebugLog << "*************** will reconnect mysql ";
    reconnect();
  }
  if (!m_sql_handler) {
    DebugLog << "reconnect error, query return -1";
    return -1;
  }

  DebugLog << "begin to excute sql[" << sql << "]";
  int rt = mysql_real_query(m_sql_handler, sql.c_str(), sql.length());
  if (rt != 0) {
    ErrorLog << "excute mysql_real_query error, sql[" << sql << "], mysql sys errinfo[" << mysql_error(m_sql_handler) << "]"; 
    // if connect error, begin to reconnect
    if (mysql_errno(m_sql_handler) == CR_SERVER_GONE_ERROR || mysql_errno(m_sql_handler) == CR_SERVER_LOST) {
      
      rt = reconnect();
      if (rt != 0 && !m_in_trans) {
        // if reconnect succ, and current is not a trans, can do query sql again 
        rt = mysql_real_query(m_sql_handler, sql.c_str(), sql.length());
        return rt;
      }
    }
  } else {
    InfoLog << "excute mysql_real_query success, sql[" << sql << "]";
  }
  return rt;
}

MYSQL_RES* MySQLInstase::storeResult() {
  if (!m_init_succ) {
    ErrorLog << "query error, mysql_handler init faild";
    return NULL;
  }
  int count = mysql_field_count(m_sql_handler);
  if (count != 0) {
    MYSQL_RES* res = mysql_store_result(m_sql_handler);
    if (!res) {
      ErrorLog << "excute mysql_store_result error, mysql sys errinfo[" << mysql_error(m_sql_handler) << "]";
    } else {
      DebugLog << "excute mysql_store_result success";
    }
    return res;
  } else {
    DebugLog << "mysql_field_count = 0, not need store result";
    return NULL;
  }

}

MYSQL_ROW MySQLInstase::fetchRow(MYSQL_RES* res) {
  if (!m_init_succ) {
    ErrorLog << "query error, mysql_handler init faild";
    return NULL;
  }
  return mysql_fetch_row(res);
}

long long MySQLInstase::numFields(MYSQL_RES* res) {
  if (!m_init_succ) {
    ErrorLog << "query error, mysql_handler init faild";
    return -1;
  }
  return mysql_num_fields(res);
}

void MySQLInstase::freeResult(MYSQL_RES* res) {
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


long long MySQLInstase::affectedRows() {
  if (!m_init_succ) {
    ErrorLog << "query error, mysql_handler init faild";
    return -1;
  }
  return mysql_affected_rows(m_sql_handler);

}


std::string MySQLInstase::getMySQLErrorInfo() {
  return std::string(mysql_error(m_sql_handler));
}

int MySQLInstase::getMySQLErrno() {
  return mysql_errno(m_sql_handler);
}

#endif

}