#include "UserModel.h"
#include "db.h"
bool UserModel::insert(User &user) // 插入数据
{
    // 插入数据
    char sql[1024];
    sprintf(sql, "insert into User(name,password,state) values('%s','%s','%s')", user.getName().c_str(),
            user.getPwd().c_str(), user.getState().c_str());
    // 连接数据库
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            // 获取主键id
            user.setID(mysql_insert_id(mysql.getconnection()));
            return true;
        }
    }
    return false;
}
// 根据主键查询用户信息
User UserModel::query(int id)
{
    char sql[1024];
    sprintf(sql, "select * from User where id=%d", id);
    // 连接数据库
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr) // 查询成功
        {
            MYSQL_ROW row = mysql_fetch_row(res); // 结果集res中读取一行数据
            if (row != nullptr)
            {
                User user;
                user.setID(atoi(row[0])); // 转化为整数
                user.setName(row[1]);
                user.setPwd(row[2]);
                user.setState(row[3]);
                // 释放资源
                mysql_free_result(res);
                return user;
            }
        }
    }
    return User();
}
// 跟新用户状态
bool UserModel::updateState(User &user)
{
    char sql[1024];
    sprintf(sql, "update User set state='%s' where id=%d", user.getState().c_str(), user.getID());
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql)) // 成功
        {
            return true;
        }
    }

    return false;
}
// 重置用户状态
void UserModel::resetState()
{

    char sql[1024];
    sprintf(sql, "update User set state='offline' where state='online'");
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}