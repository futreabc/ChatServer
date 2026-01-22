#include "chatserver.h"
#include<iostream>
#include<signal.h>
#include<chatservice.h>
using namespace std;
void resetHandler(int num)//信号处理函数
{

    ChatService::instance()->reset();
}
int main(int argc,char* argv[])
{
    if(argc<3)
    {
        cerr << "command invalid! example: ./ChatServer 127.0.0.1 8888" << endl;
        exit(-1);
    }
   const char* ip=argv[1];
    int port=atoi(argv[2]);
   struct sigaction act;
   //清空位图
   sigemptyset(&act.sa_mask);
   act.sa_handler=resetHandler;
   //SA_RESTART：被信号中断的系统调用（如 read、write）自动重启；
   act.sa_flags=SA_RESTART;
    sigaction(SIGINT,&act,NULL);//注册信号
    EventLoop loop;//创建事件循环对象
    //EventLoop* loop=new EventLoop();
    InetAddress addr(ip,port);//创建服务器监听地址对象
    ChatServer server(&loop,addr,"ChatServer");//创建服务器对象
    server.start();//启动服务
    loop.loop();//启动事件循环
    return 0;
}
