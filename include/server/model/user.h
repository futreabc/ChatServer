#ifndef USER_H
#define USER_H
#include <string>
using namespace std;
// 匹配User表的ORM映射类
class User
{
public:
    User(int id = -1, string name = "", string password = "", string state = "offline")
    {
        this->id = id;
        this->name = name;
        this->password = password;
        this->state = state;
    }
    void setID(int id)
    {
        this->id = id;
    }
    void setName(string name)
    {
        this->name = name;
    }
    void setPwd(string pwd)
    {
        this->password = pwd;
    }
    void setState(string state)
    {
        this->state = state;
    }
   int getID() const 
    {
        return id;
    }
   string getName() const 
    {
        return name;
    }
  string getPwd() const 
    {
        return password;
    }
  string getState() const 
    {
        return state;
    }

protected:
    int id;
    string name;
    string password;
    string state;
};

#endif