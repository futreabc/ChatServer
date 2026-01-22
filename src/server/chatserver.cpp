#include"chatserver.h"
#include<functional>//bind函数的头文件
#include<string>
#include<iostream>
#include<jsoncpp/json/json.h>
#include<muduo/base/Logging.h>
#include "chatservice.h"
using namespace std;
using namespace placeholders;//占位符命名空间
ChatServer::ChatServer(EventLoop* loop,const InetAddress& listenAddr,const std::string& nameArg)
    :_server(loop,listenAddr,nameArg),
    _loop(loop)
{
    //注册连接回调函数
    _server.setConnectionCallback(
        std::bind(&ChatServer::onConnection,this,_1)
    );
    //注册读回调函数
    _server.setMessageCallback(
        std::bind(&ChatServer::onMessage,this,
                  _1,
                  _2,
                  _3)
    );
    //注册写完成回调函数
    _server.setWriteCompleteCallback(
        std::bind(&ChatServer::onWriteComplete,this,_1)
    );
    _server.setThreadNum(4);//设置底层subloop的个数，4个IO线程
}
void ChatServer::start()
{
    _server.start();//启动服务
}
void ChatServer::onConnection(const TcpConnectionPtr& conn)//连接，断开事件回调
{
    if(!conn->connected())
    {
        ChatService::instance()->clientCloseException(conn); 
        conn->shutdown();//清理资源
    }
}   
void ChatServer::onMessage(const TcpConnectionPtr& conn,
                            Buffer* buffer,
                            Timestamp time)//读写事件回调
{
    string buf=buffer->retrieveAllAsString();//读取数据
    //数据的反序列化
    Json::Value Jsdata;
    Json::Reader reader;
    if(!reader.parse(buf,Jsdata))
    {
        LOG_ERROR<<"json反序列失败";
    }
    //获取处理器方法
    auto Msghandler=ChatService::instance()->getHandler(Jsdata["msgid"].asInt());
    //调用处理器
    Msghandler(conn,Jsdata,time);  

    
}
void ChatServer::onWriteComplete(const TcpConnectionPtr& conn)//写完成回调
{

}
ChatServer::~ChatServer()//析构函数
{

}