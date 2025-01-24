#ifndef CHAT_SERVICE_H_
#define CHAT_SERVICE_H_

#include <unordered_map>
#include <functional>

#include <muduo/net/TcpConnection.h>
#include "json.hpp"

using MsgHandler = std::function<void(const muduo::net::TcpConnectionPtr&, 
                                        nlohmann::json&,
                                        muduo::Timestamp)>;

//聊天服务器业务类
class ChatService {
public:
    //获取单例对象的接口函数
    static ChatService* Instance();
    //登录业务
    void Login(const muduo::net::TcpConnectionPtr& conn, nlohmann::json& js, muduo::Timestamp time);
    //注册业务
    void Reg(const muduo::net::TcpConnectionPtr& conn, nlohmann::json& js, muduo::Timestamp time);
    //获取消息对应的处理器
    MsgHandler GetHandler(int msgid);
private:
    ChatService();

    //消息id和其对应事件的处理方法
    std::unordered_map<int,MsgHandler> msgHandlerMap_;
};


#endif