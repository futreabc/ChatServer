#ifndef DB_H
#define DB_H
#include <mysql/mysql.h>
#include <string>
using namespace std;
// 数据库操作
class MySQL
{
public:
    MySQL();

    ~MySQL();

    bool connect();

    // 更新操作
    bool update(string sql);
    // 查询操作
    MYSQL_RES *query(string sql);
    //获取MYSQL
    MYSQL* getconnection();
private:
    // MYSQL 结构体是 MySQL C API 操作数据库的 “核心容器”，所有和连接相关的信息都存在这里。
    // 初始化句柄
    MYSQL *_conn;
};

#endif