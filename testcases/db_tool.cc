#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

int main(int argc, char* argv[]) {
    sql::mysql::MySQL_Driver *driver;
    sql::Connection *con;
    sql::Statement *stmt;
    sql::ResultSet *res;

    // Create a MySQL driver instance
    driver = sql::mysql::get_mysql_driver_instance();

    // Establish a connection to the MySQL server
    con = driver->connect("tcp://10.53.48.90:3306", "root", "123456");

    // Select the database
    con->setSchema("entrytask");

    // Create a statement object
    stmt = con->createStatement();

    // Execute the query and retrieve the result set
    res = stmt->executeQuery("select * from userinfo where uid = 1");

    // Iterate over the result set and retrieve the row data
    while (res->next()) {
        // int id = res->getInt("id");
        std::string name = res->getString("username");
        // Retrieve other column values as needed
        // ...

        // Process the row data
        // ...
        std::cout << "selected name ="<< name << std::endl;

    }

    // Clean up resources
    delete res;
    delete stmt;
    delete con;

    return 0;
}