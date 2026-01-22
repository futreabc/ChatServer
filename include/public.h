#ifndef PUBLIC_H
#define PUBLIC_H

/*
公共文件
*/
enum EnMsgType{
    LOGIN_MSG=1,//登录消息
    LOGIN_MSG_ACK,//登录消息相应
    LOGINOUT_MSG,//注销信息
    REG_MSG,//注册消息
    REG_MSG_ACK,//注册相应消息
    ONE_CHAT_MGS,//一对一聊天
    ADD_Friend_MSG,//添加好友信息
    CREATE_GROUP_MSG,//创建群聊
    ADD_GROUP_MSG,//添加群聊
    GROUP_CHAT_MSG,//群聊天
};
#endif

