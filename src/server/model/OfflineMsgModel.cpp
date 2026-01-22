#include "OfflineMsgModel.h"
#include "db.h"
using namespace std;
// 插入
void OfflineMsgMode::insert(int userid, string msg)
{
    char sql[1024];
    sprintf(sql, "insert into OfflineMessage(userid,message) values(%d,'%s')", userid, msg.c_str());
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}
// 删除
void OfflineMsgMode::remove(int userid)
{
    char sql[1024];
    sprintf(sql, "delete from  OfflineMessage where userid=%d", userid);
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}
// 查询
std::vector<std::string> OfflineMsgMode::query(int userid)
{

    char sql[1024];
    sprintf(sql, "select * from OfflineMessage where userid=%d", userid);
    MySQL mysql;
    //存储离线消息
    vector<string>vec;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            //把用户所有的离线消息返回到vector
            MYSQL_ROW row;
            while((row=mysql_fetch_row(res))!=nullptr)
            {
                vec.push_back(row[1]);
            }
            mysql_free_result(res);
            return vec;
        }
    }
    return vec;
}