#ifndef GROUP_USER_H
#define GROUP_USER_H
#include "user.h"
//群用户成员信息，多了一个role的信息，从User继承
class GroupUser:public User
{
public:
    void setRole(string role) {this->role=role;}
    string getRole() {return role;}
private:
    //用户的角色信息
    string role;

};
#endif