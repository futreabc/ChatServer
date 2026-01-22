#include "chatservice.h"
#include "public.h"
#include <muduo/base/Logging.h>
#include <vector>

// 获取对象接口 饿汉模式
ChatService *ChatService::instance()
{
  static ChatService service;
  return &service;
}
// 注册消息以及相应的回调操作
ChatService::ChatService()
{
  // 类的成员函数暗含this指针需要绑定，绑定处理方法
  _MshandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
  _MshandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
  _MshandlerMap.insert({ONE_CHAT_MGS, std::bind(&ChatService::onechat, this, _1, _2, _3)});
  _MshandlerMap.insert({ADD_Friend_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});
  _MshandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::CreatGroup, this, _1, _2, _3)});
  _MshandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::AddGroup, this, _1, _2, _3)});
  _MshandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::GroupChat, this, _1, _2, _3)});
  _MshandlerMap.insert({LOGINOUT_MSG, std::bind(&ChatService::loginout, this, _1, _2, _3)});
  // 连接redis
  if (_redis.connect())
  {
    // 设置上报消息函数回调
    _redis.init_notify_handler(std::bind(&ChatService::handlerRedisSubsrcibeMessage, this, _1, _2));
  }
}
// 登录业务
void ChatService::login(const TcpConnectionPtr &conn, Json::Value &js, Timestamp time)
{
  // 1. 获取id和密码
  int id = js["id"].asInt();
  string pwd = js["password"].asCString();
  // 返回查询结果
  User user = _usermodel.query(id);
  if (user.getID() == id && user.getPwd() == pwd)
  {

    // 检查是否重复登录
    if (user.getState() == "online")
    {
      // 失败
      Json::Value response;
      // 注册应答
      response["msgid"] = LOGIN_MSG_ACK;
      // 失败设置错误为1
      response["errno"] = 2;
      response["errmsg"] = "用户名已经登录,不允许重复登录";
      Json::StyledWriter arr;
      string buff = arr.write(response);
      conn->send(buff);
    }
    else
    {
      // 保证线程安全
      {
        // 模板
        lock_guard<mutex> lock(_connMutex);
        // 成功,记录用户信息
        _userConnMap.insert({id, conn});
      }
      // 向通道订阅（id）
      _redis.subscribe(id);
      // 设置用户为在线状态
      user.setState("online");
      _usermodel.updateState(user);
      Json::Value response;
      // 注册应答
      response["msgid"] = LOGIN_MSG_ACK;
      // 正确错误号设置为0
      response["errno"] = 0;
      response["id"] = user.getID();
      response["name"] = user.getName();
      // 发送离线消息
      vector<string> vec = _OfflineMsgModel.query(user.getID());
      Json::Value offlineMsg;
      if (!vec.empty())
      {

        for (auto it = vec.begin(); it != vec.end(); it++)
        {
          offlineMsg.append(*it);
          // 读取以后把离线消息删除
          _OfflineMsgModel.remove(id);
        }
      }
      response["OfflineMsg"] = offlineMsg;
      // 查询用户的好友信息
      vector<User> UserVec = _FriendModel.query(id);
      Json::Value FriendMsg; // 这是一个JSON数组

      // 遍历好友列表，逐个转换为JSON对象
      for (const auto &user : UserVec)
      {                                        // 用const&避免拷贝，更高效
        Json::Value friendItem;                // 单个好友的JSON对象
        friendItem["id"] = user.getID();       // 假设User有getId()方法
        friendItem["name"] = user.getName();   // 假设User有getName()方法
        friendItem["state"] = user.getState(); // 假设User有getState()方法

        FriendMsg.append(friendItem); // 将单个好友对象添加到数组中
      }

      // 无论好友列表是否为空，都直接赋值（空数组也是合法的JSON值）
      response["friends"] = FriendMsg;
      // 查询用户的群组信息
      vector<Group> groupUserVec = _GroupModel.queryGroup(id);
      // group[{group:[xxx,xxx,xxx]}]
      Json::Value allGroup;
      if (!groupUserVec.empty())
      {
        for (Group &group : groupUserVec)
        {
          Json::Value grpjson;
          grpjson["id"] = group.getID();
          grpjson["groupname"] = group.getName();
          grpjson["groupdesc"] = group.getDesc();
          Json::Value GroupUser_arr;
          for (GroupUser &groupuser : group.getUsers())
          {
            Json::Value grpUser;
            grpUser["id"] = groupuser.getID();
            grpUser["name"] = groupuser.getName();
            grpUser["state"] = groupuser.getState();
            grpUser["role"] = groupuser.getRole();
            GroupUser_arr.append(grpUser);
          }
          grpjson["users"] = GroupUser_arr;
          allGroup.append(grpjson);
        }
        response["GroupMsg"] = allGroup;
      }

      // 发送消息
      Json::StyledWriter arr;
      string buff = arr.write(response);
      conn->send(buff);
    }
  }

  else
  {
    // 失败
    Json::Value response;
    // 注册应答
    response["msgid"] = LOGIN_MSG_ACK;
    // 失败设置错误为1
    response["errno"] = 1;
    response["errmsg"] = "用户名或者密码错误";
    Json::StyledWriter arr;
    string buff = arr.write(response);
    conn->send(buff);
  }
}
// 注册业务
void ChatService::reg(const TcpConnectionPtr &conn, Json::Value &js, Timestamp time)
{

  string name = js["name"].asString();
  string pwd = js["password"].asString();
  User user;
  user.setName(name);
  user.setPwd(pwd);
  bool state = _usermodel.insert(user);
  if (state)
  {
    // 成功
    Json::Value response;
    // 注册应答
    response["msgid"] = REG_MSG_ACK;
    // 正确错误号设置为0
    response["errno"] = 0;
    response["id"] = user.getID();
    Json::StyledWriter arr;
    string buff = arr.write(response);
    conn->send(buff);
  }
  else
  {
    // 失败
    Json::Value response;
    // 注册应答
    response["msgid"] = REG_MSG_ACK;
    // 失败设置错误为1
    response["errno"] = 1;
    response["errmsg"] = "注册失败";
    Json::StyledWriter arr;
    string buff = arr.write(response);
    conn->send(buff);
  }
}
// 获取处理器
MsgHandler ChatService::getHandler(int Msgid)
{
  // 记录错误日志，没有找到对应的处理器
  auto it = _MshandlerMap.find(Msgid);
  if (it == _MshandlerMap.end())
  {
    // 返回一个默认的处理器，不处理
    return [=](const TcpConnectionPtr &conn, Json::Value &js, Timestamp time)
    {
      LOG_ERROR << "Msgid " << Msgid << "Not found handler!";
    };
  }
  else
  {
    return _MshandlerMap[Msgid];
  }
}
// 异常错误处理
void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
  User user;
  {
    lock_guard<mutex> lock(_connMutex);
    for (auto it = _userConnMap.begin(); it != _userConnMap.end(); it++)
    {
      if (it->second == conn)
      {
        // 从map表中删除用户的连接信息
        _userConnMap.erase(it);
        user.setID(it->first);
        break;
      }
    }
  }
  // 在redis中取消订阅
  _redis.unsubscribe(user.getID());
  if (user.getID() != -1)
  {
    user.setState("offline");
    _usermodel.updateState(user);
  }
}
// 一对一聊天业务
void ChatService::onechat(const TcpConnectionPtr &conn, Json::Value &js, Timestamp time)
{
  int toid = js["to"].asInt();
  {
    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(toid);
    if (it != _userConnMap.end())
    {
      Json::StyledWriter arr;
      it->second->send(arr.write(js));
      return;
    }
  }
  // 查询id是否在线
  User user = _usermodel.query(toid);
  if (user.getState() == "online")
  {
    Json::StyledWriter Write;
    _redis.publish(toid, Write.write(js));
    return;
  }
  // 用户不在线
  Json::StyledWriter arr;
  _OfflineMsgModel.insert(toid, arr.write(js));
}
//{"msgid":1,"id":1,"password":"123456"}
//{"msgid":5,"to":2,"msg":"hello"}
//{"msgid":6,"id":2,"friendid":1"}
// 业务重置，服务器异常
void ChatService::reset()
{
  _usermodel.resetState();
}
// 添加好友业务
//  msgid id friendid
void ChatService::addFriend(const TcpConnectionPtr &conn, Json::Value &js, Timestamp time)
{
  int userid = js["id"].asInt();
  int friendid = js["friendid"].asInt();
  // 存储好友
  _FriendModel.insert(userid, friendid);
}
// 创建群聊  id,groupname,groupdesc
void ChatService::CreatGroup(const TcpConnectionPtr &conn, Json::Value &js, Timestamp time)
{
  int userid = js["id"].asInt();
  string groupName = js["groupname"].asString();
  string groupDesc = js["groupdesc"].asString();
  Group group(-1, groupName, groupDesc);
  if (_GroupModel.createGroup(group))
  {
    // 存储群组创建人信息
    _GroupModel.addGroup(userid, group.getID(), "creator");
  }
}
// 添加群聊 id groupid
void ChatService::AddGroup(const TcpConnectionPtr &conn, Json::Value &js, Timestamp time)
{
  int userid = js["id"].asInt();
  int groupid = js["groupid"].asInt();
  _GroupModel.addGroup(userid, groupid, "normal");
}
// 进行群聊天
void ChatService::GroupChat(const TcpConnectionPtr &conn, Json::Value &js, Timestamp time)
{
  int userid = js["id"].asInt();
  int groupid = js["groupid"].asInt();
  vector<int> Useridvec = _GroupModel.queryGroupUsers(userid, groupid);
  lock_guard<mutex> lock(_connMutex);
  for (int id : Useridvec)
  {
    auto it = _userConnMap.find(id);
    if (it != _userConnMap.end())
    {
      // 转发群消息
      Json::StyledWriter arr;
      it->second->send(arr.write(js));
    }
    else
    {
      // 查询id是否在线
      User user = _usermodel.query(id);
      if (user.getState() == "online")
      {
        Json::StyledWriter Write;
        _redis.publish(id, Write.write(js));
      }
      // 不在线，保存离线消息
      else
      {
        Json::StyledWriter arr;
        _OfflineMsgModel.insert(id, arr.write(js));
      }
    }
  }
}
// 注销业务
void ChatService::loginout(const TcpConnectionPtr &conn, Json::Value &js, Timestamp time)
{
  int userid = js["id"].asInt();
  {
    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(userid);
    if (it != _userConnMap.end())
    {
      _userConnMap.erase(it);
    }
  }
  // 用户注销，相当于就是下线，在redis中取消订阅通道
  _redis.unsubscribe(userid);
  // 更新用户的状态信息
  User user(userid, "", "", "offline");
  _usermodel.updateState(user);
}
void ChatService::handlerRedisSubsrcibeMessage(int channel, string Message)
{
  // Json::Reader reader;
  // Json::Value response;
  // if(!reader.parse(Message,response))
  // {
  //  LOG_ERROR<<"json反序列失败";
  // }
  lock_guard<mutex> lock(_connMutex);
  auto it=_userConnMap.find(channel);
  if(it!=_userConnMap.end())
  {
    it->second->send(Message);
    return;
  }
//存储离校消息
  _OfflineMsgModel.insert(channel,Message);
}