#ifndef CHAT_SERVICE_H_
#define CHAT_SERVICE_H_

#include <unordered_map>
#include <functional>
#include <mutex>

#include <muduo/net/TcpConnection.h>
#include "json.hpp"

#include "user_model.h"

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
    //一对一聊天业务
    void OneChat(const muduo::net::TcpConnectionPtr& conn, nlohmann::json& js, muduo::Timestamp time);
    //获取消息对应的处理器
    MsgHandler GetHandler(int msgid);
    //处理客户端异常退出
    void ClientCloseException(const muduo::net::TcpConnectionPtr conn);
private:
    ChatService();

    //消息id和其对应事件的处理方法
    std::unordered_map<int,MsgHandler> msg_handler_map_;

    //存储在线用户通信连接,对它的访问应当注意线程安全
    std::unordered_map<int,muduo::net::TcpConnectionPtr> user_conn_map_;
    std::mutex mutex_;

    //数据操作类对象
    UserModel user_model_;
};


#endif