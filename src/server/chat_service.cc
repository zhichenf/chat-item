#include "chat_service.h"

#include <muduo/base/Logging.h>

#include "public.h"

// using MsgHandler = std::function<void(const muduo::net::TcpConnectionPtr&, 
//                                         nlohmann::json&,
//                                         muduo::Timestamp)>;

// //聊天服务器业务类
// class ChatService {
// public:
//     //获取单例对象的接口函数
//     static ChatService* Instance();
//     //登录业务
//     void Login(const muduo::net::TcpConnectionPtr& conn, nlohmann::json& js, muduo::Timestamp time);
//     //注册业务
//     void Reg(const muduo::net::TcpConnectionPtr& conn, nlohmann::json& js, muduo::Timestamp time);
//     //获取消息对应的处理器
//     MsgHandler GetHandler(int msgid);
// private:
//     ChatService();

//     //消息id和其对应事件的处理方法
//     std::unordered_map<int,MsgHandler> msgHandlerMap_;
// };

//获取单例对象的接口函数
ChatService* ChatService::Instance() {
    static ChatService service;
    return &service;
}

//注册消息以及对应的回调操作
ChatService::ChatService() {
    using namespace std::placeholders;
    msgHandlerMap_.insert({kLoginMsg,std::bind(&ChatService::Login, this, _1, _2, _3)});
    msgHandlerMap_.insert({kRegMsg,std::bind(&ChatService::Reg, this, _1, _2, _3)});
}

MsgHandler ChatService::GetHandler(int msgid){
    auto it = msgHandlerMap_.find(msgid);
    if (msgHandlerMap_.end() == it) {
        return [=](const muduo::net::TcpConnectionPtr&, nlohmann::json&,muduo::Timestamp) {
            LOG_ERROR << "msgid:" << msgid << " can not find handler!";
        };
    }
    return msgHandlerMap_[msgid];
}

void ChatService::Login(const muduo::net::TcpConnectionPtr& conn, nlohmann::json& js, muduo::Timestamp time) {
    LOG_INFO << "do login service!!";
}

void ChatService::Reg(const muduo::net::TcpConnectionPtr& conn, nlohmann::json& js, muduo::Timestamp time) {
    LOG_INFO << "do reg service!!";
}


