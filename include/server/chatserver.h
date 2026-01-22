#ifndef CHATSERVER_H
#define CHATSERVER_H
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
using namespace muduo::net;
using namespace muduo;
class ChatServer{
    public:
        ChatServer(EventLoop* loop,const InetAddress& listenAddr,const std::string& nameArg);//初始化
        ~ChatServer();
        void start();//启动服务 
    private:
        TcpServer _server;//CP 服务的核心封装（负责监听、连接管理）
        EventLoop* _loop;//IO线程，事件循环，必须是指针，一个线程对应一个事件循环
        void onMessage(const TcpConnectionPtr&,
                            Buffer*,
                            Timestamp);//读事件回调
        void onWriteComplete(const TcpConnectionPtr&);//写完成事件回调
        void onConnection(const TcpConnectionPtr&);//连接建立或断开时的回调
};   
#endif