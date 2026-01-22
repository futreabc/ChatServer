#ifndef GROUP_H
#define GROUP_H
#include<string>
#include<vector>
#include "groupUser.h"
class Group
{
public:
    Group(int id=-1,string name="",string desc="")
    {
        this->id=id;
        this->name=name;
        this->desc=desc;
    }
    void setID(int id) {this->id=id;}
    void setName(string name) {this->name=name;}
    void setDesc(string desc) {this->desc=desc;}
    int getID() {return id;}
    string getName() {return name;}
    string getDesc() {return desc;}
    vector<GroupUser>& getUsers() {return users;}
private:
    int id;//组id
    std::string name;//组名字
    std::string desc;//组的功能
    std::vector<GroupUser> users;//组的成员信息
};
#endif
