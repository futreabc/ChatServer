#ifndef CHATSERVICE_H
#define CHATSERVICE_H
#include<unordered_map>
#include <muduo/net/TcpConnection.h>
#include<functional>
#include<jsoncpp/json/json.h>
#include "UserModel.h"
#include<mutex>
#include "OfflineMsgModel.h"
#include "FriendModel.h"
#include "groupModel.h"
#include "redis.h"
using namespace std;
using namespace muduo::net;
using namespace muduo;
//相当于加强版的函数指针（仿函数，自动推导，都可以）,处理消息回调类型,函数指针
using MsgHandler=std::function<void(const TcpConnectionPtr& conn,Json::Value& js,Timestamp time)>;
//单例模式,业务类 
class ChatService
{

    public:
    //获取单例模式对象的接口
    static ChatService* instance();
    //登录业务
    void login(const TcpConnectionPtr& conn,Json::Value& js,Timestamp time);
    //注册业务
    void reg(const TcpConnectionPtr& conn,Json::Value& js,Timestamp time);
    //获取消息的处理器
    MsgHandler getHandler(int Msgid);
    //客户端异常处理
    void clientCloseException(const TcpConnectionPtr&conn);
    //一对一聊天
    void onechat(const TcpConnectionPtr& conn,Json::Value& js,Timestamp time);
    //服务器异常，业务重置
    void reset();
    //添加好友消息
    void addFriend(const TcpConnectionPtr& conn,Json::Value& js,Timestamp time);
    //创建群聊
    void CreatGroup(const TcpConnectionPtr& conn,Json::Value& js,Timestamp time);
    //添加群聊
    void AddGroup(const TcpConnectionPtr& conn,Json::Value& js,Timestamp time);
    //群聊天
    void GroupChat(const TcpConnectionPtr& conn,Json::Value& js,Timestamp time);
    //注销业务
    void loginout(const TcpConnectionPtr& conn,Json::Value& js,Timestamp time);
    //处理redis上报函数回调
    void handlerRedisSubsrcibeMessage(int,string);
    private:
    //存储消息id和处理方法
    unordered_map<int,MsgHandler> _MshandlerMap;
    ChatService();
    //数据操作类对象
    UserModel _usermodel;
    //存储在线用户的通信连接
    unordered_map<int,TcpConnectionPtr>_userConnMap;
    //互斥锁(用于保证_userConnMap的线程安全)
    mutex _connMutex;
    //离线消息操作对象
    OfflineMsgMode _OfflineMsgModel;
    //好友表操作对象
    FriendModel _FriendModel;
    //群消息操作对象
    GroupModel _GroupModel;
    //redis操作对象
    Redis _redis;

};
#endif