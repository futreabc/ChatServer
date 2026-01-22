#ifndef GROUPMODEL_H
#define GROUPMODEL_H
#include "group.h"
class GroupModel
{
public:
    //创建群聊
    bool createGroup(Group& group);
    //添加群组
    void addGroup(int userid,int groupid,string role);
    //查询用户所在群消息
    vector<Group> queryGroup(int userid);
    //根据指定的groupid查询群用户id列表，除了userid自己，用于群聊
    vector<int>  queryGroupUsers(int userid,int groupid);
};
#endif