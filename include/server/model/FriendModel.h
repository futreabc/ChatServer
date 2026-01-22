#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H
#include<vector>
#include<user.h>
//提供好友表的信息操作方法
class FriendModel
{
public:
    //添加好友
    void insert(int userid,int friendid);
    //返回好友列表信息
    std::vector<User> query(int userid);//联合查询
};



#endif