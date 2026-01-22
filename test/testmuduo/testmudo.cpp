#include<iostream>
#include<string>
#include<muduo/net/TcpServer.h>
#include<muduo/net/TcpClient.h>
#include<muduo/net/EventLoop.h>
#include<functional>// 必须包含bind的头文件
using namespace std;
using namespace muduo;
using namespace muduo::net;

class ChatServer//自定义服务器类
{
private:
    /* data */
    TcpServer  _server;//服务器封装,
    EventLoop* _loop;//事件循环，
public:
    ChatServer(EventLoop* loop,const InetAddress& listenAddr,const string& nameArg):_server(loop,listenAddr,nameArg),_loop(loop)
    {
        //设置连接和断开回调
        _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, std::placeholders::_1));
        //读事件和写事件回调
        _server.setMessageCallback(std::bind(&ChatServer::onMessage,this,_1,_2,_3));
        //设置线程的数量,moduo会自己分配I/O线程和工作线程
        _server.setThreadNum(2);
    }
    ~ChatServer()
    {
        
    }
    /*会封装一个 TCP 连接的所有信息（比如 fd、对方 IP / 端口、连接状态），
    通过智能指针传递能避免野指针，保证连接对象的生命周期安全。*/
    //typedef std::function<void (const TcpConnectionPtr&)> ConnectionCallback;
    void onConnection(const TcpConnectionPtr&conn)//回调函数，本质是指向 “TCP 连接对象” 的智能指针
    {
        if(conn->connected())//连接状态
        {
            cout<<conn->peerAddress().toIpPort()<<"->"<<conn->localAddress().toIpPort()<<endl;
        }
        else//断开状态
        {
            cout<<conn->peerAddress().toIpPort()<<"->"<<conn->localAddress().toIpPort()<<endl;
            cout<<"连接断开"<<endl;
            conn->shutdown();//close(fd);
        }
    }
    void onMessage(const TcpConnectionPtr&conn,//连接,读事件回调（OnMessage）：有数据可读时触发
                            Buffer*buff,//缓冲区
                            Timestamp time)//时间，标记 “写事件就绪的具体时间点
                            {
                                string buf=buff->retrieveAllAsString();//读出全部数据
                                cout<<"recv data is "<<buf<<"time is "<<time.toString()<<endl;
                                conn->send(buf);//发数据

                            }  

    void start()
    {
        _server.start();//开启工作线程的事件循环，关联主线程的事件循环
    }
};
/*
// 2. 写事件回调（WriteComplete）：可写时触发
using WriteCallback = std::function<void (
    const TcpConnectionPtr&,    // 连接指针
    Timestamp                   // 写事件就绪时间
)>;
*/
int main()
{
EventLoop loop;//主事件循环,while+epoll
InetAddress addr("127.0.0.1",6000);
ChatServer Server(&loop,addr,"ChatServer");
Server.start();//listenfd 添加到 epoll
loop.loop();//开启事件循环(阻塞)


    return 0;
}