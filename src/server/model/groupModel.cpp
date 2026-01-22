#include "groupModel.h"
#include "db.h"
#include<cstring>
// 创建群聊
bool GroupModel::createGroup(Group &group)
{
    char sql[1024]={0};
    sprintf(sql, "insert into AllGroup(groupname,groupdesc) values('%s','%s')", group.getName().c_str(), group.getDesc().c_str());
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            // 返回一下组id
            group.setID(mysql_insert_id(mysql.getconnection()));
            return true;
        }
    }
    return false;
}
// 添加群组
void GroupModel::addGroup(int userid, int groupid, string role)
{
    char sql[1024]={0};
    sprintf(sql, "insert into GroupUser values(%d,%d,'%s')", groupid, userid, role.c_str());
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
    return;
}
// 查询用户所在群消息
vector<Group> GroupModel::queryGroup(int userid)
{
    char sql[1024]={0};
    sprintf(sql, "select a.id,a.groupname,a.groupdesc from AllGroup a inner join GroupUser b on a.id=b.groupid where b.userid=%d", userid);
    MySQL mysql;
    vector<Group> vec;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                Group group;
                group.setID(atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);
                vec.push_back(group);
            }
            mysql_free_result(res);
        }
    }
    for (Group &group : vec)
    {
        memset(sql,'\0',sizeof(sql));
        sprintf(sql, "select a.id,a.name,a.state,b.grouprole from User a inner join GroupUser b on a.id=b.userid where b.groupid=%d", group.getID());
        MYSQL_RES* res=mysql.query(sql);
        if(res!=nullptr)
        {
            MYSQL_ROW row;
            while((row=mysql_fetch_row(res))!=nullptr)
            {
                    GroupUser groupuser;
                    groupuser.setID(atoi(row[0]));
                    groupuser.setName(row[1]);
                    groupuser.setState(row[2]);
                    groupuser.setRole(row[3]);
                    group.getUsers().push_back(groupuser);

            }
            mysql_free_result(res);
        }
    }
    return vec;
}
// 根据指定的groupid查询群用户id列表，除了userid自己，用于群聊
vector<int> GroupModel::queryGroupUsers(int userid, int groupid)
{
    char sql[1024]={0};
    sprintf(sql,"select userid from GroupUser where groupid=%d and userid!=%d",groupid,userid);
    MySQL mysql;
    vector<int> vec;
    if(mysql.connect())
    {
        MYSQL_RES* res=mysql.query(sql);
        if(res!=nullptr)
        {
            MYSQL_ROW row;
            while((row=mysql_fetch_row(res))!=nullptr)
            {
              vec.push_back(stoi(row[0]));
            }
            mysql_free_result(res);
        }
    }
    return vec;
}