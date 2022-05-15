#ifndef TINYRPC_COMM_MYSQL_INSTASE_H
#define TINYRPC_COMM_MYSQL_INSTASE_H

#include <mysql/mysql.h>
#include <memory>
#include <map>
#include "tinyrpc/net/mutex.h"
#include "tinyrpc/net/net_address.h"

namespace tinyrpc {

struct MySQLOption {
 public:
  explicit MySQLOption(const IPAddress& addr) : m_addr(addr) {};
  ~MySQLOption() {};

 public:
  IPAddress m_addr;
  std::string m_user;
  std::string m_passwd;
  std::string m_select_db;
  std::string m_char_set;
};

class MySQLInstase {
 public:

  typedef std::shared_ptr<MySQLInstase> ptr;

  MySQLInstase(const MySQLOption& option);
  ~MySQLInstase();

  bool isInitSuccess();

  int Query(const std::string& sql);

  int Commit();

  MYSQL_RES* StoreResult();

  MYSQL_ROW FetchRow(MYSQL_RES* res);

  void FreeResult(MYSQL_RES* res);

  long long NumFields(MYSQL_RES* res);

  long long AffectedRows();


 private:
  bool m_init_succ {false};
  Mutex m_mutex;
  MYSQL m_sql_handler;

};


class MySQLInstaseFactroy {
 public:
  MySQLInstaseFactroy() = default;
  ~MySQLInstaseFactroy() = default;

  MySQLInstase* GetMySQLInstase(const std::string& key);
 public:
  static MySQLInstaseFactroy* GetThreadMySQLFactory();
  

 private:
  std::map<std::string, MySQLInstase::ptr> m_conn_pools;

};



}



#endif