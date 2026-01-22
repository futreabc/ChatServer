#include <jsoncpp/json/json.h>
#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <unordered_map>
#include <functional>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <atomic>

#include "group.h"
#include "user.h"
#include "public.h"

using namespace std;

// ================= 全局变量 =================
//存放用户信息，比如id和名字
User g_currentUser;

vector<User> g_currentUserFriendList;
vector<Group> g_currentUserGroupList;
bool isMainMenuRunning = false;
sem_t rwsem;
atomic_bool g_isLoginSuccess{false};

// ================= 函数声明 =================

void readTaskHandler(int clientfd);
string getCurrentTime();
void mainMenu(int);
void showCurrentUserData();

// ================= 主函数 =================

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        cerr << "command invalid! example: ./ChatClient 127.0.0.1 8888" << endl;
        exit(-1);
    }

    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == clientfd)
    {
        cerr << "socket create error" << endl;
        exit(-1);
    }

    sockaddr_in server;
    memset(&server, 0, sizeof(sockaddr_in));

    server.sin_family = AF_INET;
    //网路地址是大端序
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    if (-1 == connect(clientfd, (sockaddr *)&server, sizeof(sockaddr_in)))
    {
        cerr << "connect server error" << endl;
        close(clientfd);
        exit(-1);
    }
    //初始化信号量为0
    sem_init(&rwsem, 0, 0);
    //启动读线程（堵塞接收服务器的回发信息）
    std::thread readTask(readTaskHandler, clientfd); 
    //线程的分离
    readTask.detach(); 

    for (;;)
    {
        cout << "========================" << endl;
        cout << "1. login" << endl;
        cout << "2. register" << endl;
        cout << "3. quit" << endl;
        cout << "========================" << endl;
        cout << "choice:";
        int choice = 0;
        cin >> choice;
        cin.get(); 

        switch (choice)
        {
        case 1: // login业务
        {
            int id = 0;
            char pwd[50] = {0};
            cout << "userid:";
            cin >> id;
            cin.get(); 
            cout << "userpassword:";
            cin.getline(pwd, 50);

            Json::Value js;
            js["msgid"] = LOGIN_MSG;
            js["id"] = id;
            js["password"] = pwd;
            
            Json::FastWriter writer;
            string request = writer.write(js);

            g_isLoginSuccess = false;

            int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
            if (len == -1)
            {
                cerr << "send login msg error:" << request << endl;
            }
            //堵塞信号量
            sem_wait(&rwsem); 
                
            if (g_isLoginSuccess) 
            {
                isMainMenuRunning = true;
                mainMenu(clientfd);
            }
        }
        break;
        case 2: // register业务
        {
            char name[50] = {0};
            char pwd[50] = {0};
            cout << "username:";
            cin.getline(name, 50);
            cout << "userpassword:";
            cin.getline(pwd, 50);

            Json::Value js;
            js["msgid"] = REG_MSG;
            js["name"] = name;
            js["password"] = pwd;
            
            Json::FastWriter writer;
            string request = writer.write(js);

            int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
            if (len == -1)
            {
                cerr << "send reg msg error:" << request << endl;
            }
            
            sem_wait(&rwsem); 
        }
        break;
        case 3: // quit业务
            close(clientfd);
            sem_destroy(&rwsem);
            exit(0);
        default:
            cerr << "invalid input!" << endl;
            break;
        }
    }

    return 0;
}

// ================= 业务处理函数 =================

void doRegResponse(Json::Value &responsejs)
{
    if (0 != responsejs["errno"].asInt()) 
    {
        cerr << "name is already exist, register error!" << endl;
    }
    else 
    {
        cout << "name register success, userid is " << responsejs["id"].asInt()
                << ", do not forget it!" << endl;
    }
}

// 【关键修改】处理登录的响应逻辑
void doLoginResponse(Json::Value &responsejs)
{
    if (0 != responsejs["errno"].asInt()) // 登录失败
    {
        cerr << responsejs["errmsg"].asString() << endl;
        g_isLoginSuccess = false;
    }
    else // 登录成功
    {
        g_currentUser.setID(responsejs["id"].asInt());
        g_currentUser.setName(responsejs["name"].asString());

        // 1. 处理好友列表
        // 修正：服务端直接发送的是Json Object数组，不是序列化后的字符串数组
        if (responsejs.isMember("friends"))
        {
            g_currentUserFriendList.clear();

            Json::Value friends = responsejs["friends"];
            for (unsigned int i = 0; i < friends.size(); ++i)
            {
                // 直接通过 friends[i] 访问 Json 对象，无需再次反序列化
                Json::Value js = friends[i]; 

                User user;
                user.setID(js["id"].asInt());
                user.setName(js["name"].asString());
                user.setState(js["state"].asString());
                g_currentUserFriendList.push_back(user);
            }
        }

        // 2. 处理群组列表
        // 修正：服务端发送的Key是 "GroupMsg"，且结构是 Json Object 数组
        if (responsejs.isMember("GroupMsg")) 
        {
            g_currentUserGroupList.clear();

            Json::Value groups = responsejs["GroupMsg"];
            for (unsigned int i = 0; i < groups.size(); ++i)
            {
                // 直接解析 group json对象
                Json::Value grpjs = groups[i]; 

                Group group;
                group.setID(grpjs["id"].asInt());
                group.setName(grpjs["groupname"].asString());
                group.setDesc(grpjs["groupdesc"].asString());

                Json::Value users = grpjs["users"];
                for (unsigned int j = 0; j < users.size(); ++j)
                {
                    // 直接解析 group user json对象
                    Json::Value js = users[j]; 
                    
                    GroupUser user;
                    user.setID(js["id"].asInt());
                    user.setName(js["name"].asString());
                    user.setState(js["state"].asString());
                    user.setRole(js["role"].asString());
                    group.getUsers().push_back(user);
                }

                g_currentUserGroupList.push_back(group);
            }
        }

        // 显示登录用户的基本信息
        showCurrentUserData();

        // 3. 处理离线消息
        // 修正：服务端发送的Key是 "OfflineMsg"
        // 注意：离线消息服务端确实是存的序列化字符串，所以这里保持 Double Parse 逻辑
        if (responsejs.isMember("OfflineMsg")) 
        {
            Json::Value offlinemsg = responsejs["OfflineMsg"];
            for (unsigned int i = 0; i < offlinemsg.size(); ++i)
            {
                string str = offlinemsg[i].asString();
                Json::Reader reader;
                Json::Value js;
                reader.parse(str, js); // 反序列化具体的离线消息

                int msgtype = js["msgid"].asInt();

                if (ONE_CHAT_MGS == msgtype)
                {
                    cout << js["time"].asString() << " [" << js["id"].asInt() << "]" << js["name"].asString()
                            << " said: " << js["msg"].asString() << endl;
                }
                else if (GROUP_CHAT_MSG == msgtype)
                {
                     cout << "群消息[" << js["groupid"].asInt() << "]:" << js["time"].asString() << " [" << js["id"].asInt() << "]" << js["name"].asString()
                            << " said: " << js["msg"].asString() << endl;
                }
            }
        }

        g_isLoginSuccess = true;
    }
}

void readTaskHandler(int clientfd)
{
    for (;;)
    {
        char buffer[1024] = {0};
        int len = recv(clientfd, buffer, 1024, 0); 
        if (-1 == len || 0 == len)
        {
            close(clientfd);
            exit(-1);
        }

        Json::Reader reader;
        Json::Value js;
        if (!reader.parse(buffer, js))
        {
            cerr << "parse json error, buffer:" << buffer << endl;
            continue;
        }

        int msgtype = js["msgid"].asInt();
        if (ONE_CHAT_MGS== msgtype)
        {
            cout << js["time"].asString() << " [" << js["id"].asInt() << "]" << js["name"].asString()
                 << " said: " << js["msg"].asString() << endl;
            continue;
        }

        if (GROUP_CHAT_MSG == msgtype)
        {
            cout << "群消息[" << js["groupid"].asInt() << "]:" << js["time"].asString() << " [" << js["id"].asInt() << "]" << js["name"].asString()
                 << " said: " << js["msg"].asString() << endl;
            continue;
        }

        if (LOGIN_MSG_ACK == msgtype)
        {
            doLoginResponse(js); 
            //信号量+1
            sem_post(&rwsem);    
            continue;
        }

        if (REG_MSG_ACK == msgtype)
        {
            doRegResponse(js);
            sem_post(&rwsem);    
            continue;
        }
    }
}

void showCurrentUserData()
{
    cout << "======================login user======================" << endl;
    cout << "current login user => id:" << g_currentUser.getID() << " name:" << g_currentUser.getName() << endl;
    cout << "----------------------friend list---------------------" << endl;
    if (!g_currentUserFriendList.empty())
    {
        for (User &user : g_currentUserFriendList)
        {
            cout << user.getID() << " " << user.getName() << " " << user.getState() << endl;
        }
    }
    cout << "----------------------group list----------------------" << endl;
    if (!g_currentUserGroupList.empty())
    {
        for (Group &group : g_currentUserGroupList)
        {
            cout << group.getID() << " " << group.getName() << " " << group.getDesc() << endl;
            for (GroupUser &user : group.getUsers())
            {
                cout << user.getID() << " " << user.getName() << " " << user.getState()
                     << " " << user.getRole() << endl;
            }
        }
    }
    cout << "======================================================" << endl;
}

// ================= 命令处理模块 =================

void help(int fd = 0, string str = "");
void chat(int, string);
void addfriend(int, string);
void creategroup(int, string);
void addgroup(int, string);
void groupchat(int, string);
void loginout(int, string);

unordered_map<string, string> commandMap = {
    {"help", "显示所有支持的命令,格式help"},
    {"chat", "一对一聊天,格式chat:friendid:message"},
    {"addfriend", "添加好友,格式addfriend:friendid"},
    {"creategroup", "创建群组,格式creategroup:groupname:groupdesc"},
    {"addgroup", "加入群组,格式addgroup:groupid"},
    {"groupchat", "群聊,格式groupchat:groupid:message"},
    {"loginout", "注销,格式loginout"}};

unordered_map<string, function<void(int, string)>> commandHandlerMap = {
    {"help", help},
    {"chat", chat},
    {"addfriend", addfriend},
    {"creategroup", creategroup},
    {"addgroup", addgroup},
    {"groupchat", groupchat},
    {"loginout", loginout}};

void mainMenu(int clientfd)
{
    help();

    char buffer[1024] = {0};
    while (isMainMenuRunning)
    {
        cin.getline(buffer, 1024);
        string commandbuf(buffer);
        string command; 
        int idx = commandbuf.find(":");
        if (-1 == idx)
        {
            command = commandbuf;
        }
        else
        {
            command = commandbuf.substr(0, idx);
        }
        auto it = commandHandlerMap.find(command);
        if (it == commandHandlerMap.end())
        {
            cerr << "invalid input command!" << endl;
            continue;
        }

        it->second(clientfd, commandbuf.substr(idx + 1, commandbuf.size() - idx));
    }
}

void help(int, string)
{
    cout << "show command list >>> " << endl;
    for (auto &p : commandMap)
    {
        cout << p.first << " : " << p.second << endl;
    }
    cout << endl;
}

void addfriend(int clientfd, string str)
{
    int friendid = atoi(str.c_str());
    
    Json::Value js;
    js["msgid"] = ADD_Friend_MSG;
    js["id"] = g_currentUser.getID();
    js["friendid"] = friendid;
    
    Json::FastWriter writer;
    string buffer = writer.write(js);

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send addfriend msg error -> " << buffer << endl;
    }
}

// 【关键修改】 chat command handler
void chat(int clientfd, string str)
{
    int idx = str.find(":"); // friendid:message
    if (-1 == idx)
    {
        cerr << "chat command invalid!" << endl;
        return;
    }

    int friendid = atoi(str.substr(0, idx).c_str());
    string message = str.substr(idx + 1, str.size() - idx);

    Json::Value js;
    js["msgid"] = ONE_CHAT_MGS;
    js["id"] = g_currentUser.getID();
    js["name"] = g_currentUser.getName();
    
    // 修正：服务端 ChatService::onechat 中使用的是 js["to"]
    js["to"] = friendid; 
    
    js["msg"] = message;
    js["time"] = getCurrentTime();
    
    Json::FastWriter writer;
    string buffer = writer.write(js);

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send chat msg error -> " << buffer << endl;
    }
}

void creategroup(int clientfd, string str)
{
    int idx = str.find(":");
    if (-1 == idx)
    {
        cerr << "creategroup command invalid!" << endl;
        return;
    }

    string groupname = str.substr(0, idx);
    string groupdesc = str.substr(idx + 1, str.size() - idx);

    Json::Value js;
    js["msgid"] = CREATE_GROUP_MSG;
    js["id"] = g_currentUser.getID();
    js["groupname"] = groupname;
    js["groupdesc"] = groupdesc;
    
    Json::FastWriter writer;
    string buffer = writer.write(js);

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send creategroup msg error -> " << buffer << endl;
    }
}

void addgroup(int clientfd, string str)
{
    int groupid = atoi(str.c_str());
    
    Json::Value js;
    js["msgid"] = ADD_GROUP_MSG;
    js["id"] = g_currentUser.getID();
    js["groupid"] = groupid;
    
    Json::FastWriter writer;
    string buffer = writer.write(js);

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send addgroup msg error -> " << buffer << endl;
    }
}

void groupchat(int clientfd, string str)
{
    int idx = str.find(":");
    if (-1 == idx)
    {
        cerr << "groupchat command invalid!" << endl;
        return;
    }

    int groupid = atoi(str.substr(0, idx).c_str());
    string message = str.substr(idx + 1, str.size() - idx);

    Json::Value js;
    js["msgid"] = GROUP_CHAT_MSG;
    js["id"] = g_currentUser.getID();
    js["name"] = g_currentUser.getName();
    js["groupid"] = groupid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    
    Json::FastWriter writer;
    string buffer = writer.write(js);

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send groupchat msg error -> " << buffer << endl;
    }
}

void loginout(int clientfd, string)
{
    Json::Value js;
    js["msgid"] = LOGINOUT_MSG;
    js["id"] = g_currentUser.getID();
    
    Json::FastWriter writer;
    string buffer = writer.write(js);

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send loginout msg error -> " << buffer << endl;
    }
    else
    {
        isMainMenuRunning = false;
    }   
}

string getCurrentTime()
{
    auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm *ptm = localtime(&tt);
    char date[60] = {0};
    sprintf(date, "%d-%02d-%02d %02d:%02d:%02d",
            (int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
            (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
    return std::string(date);
}