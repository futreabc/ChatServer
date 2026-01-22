#ifndef USER_MODEL_H
#define USER_MODEL_H
#include "user.h"
//User的数据操作类
class UserModel
{
private:
    /* data */
public:
    //插入操作
    bool insert(User& user);
    //根据主键查询用户信息
    User query(int id);
    //跟新用户状态
    bool updateState(User& user);
    //重置用户状态
    void resetState();
};

#endif