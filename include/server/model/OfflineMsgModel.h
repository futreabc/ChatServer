#ifndef OFFLINEMSGMODEL_H
#define OFFLINEMSGMODEL_H
#include <string>
#include <vector>
class OfflineMsgMode
{

public:
    void insert(int userid, std::string msg);
    void remove(int userid);
    //查询离线消息
    std::vector<std::string> query(int userid);
};
 
#endif