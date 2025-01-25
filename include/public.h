#ifndef PUBLIC_H_
#define PUBLIC_H_

/*
*server 和 client的公共文件
*/

enum EnMsgType {
    kLoginMsg = 1,      //登录消息
    kLoginMsgAck,       //登录回应
    kRegMsg,            //注册消息
    kRegMsgAck,         //注册回应
    kOneChatMsg         //1对1聊天消息
};

#endif