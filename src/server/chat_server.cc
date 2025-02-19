#include "chat_server.h"

#include <functional>

#include "json.hpp"

#include "chat_service.h"

//初始化聊天服务器
ChatServer::ChatServer(muduo::net::EventLoop* loop,
             const muduo::net::InetAddress& listenAddr,
             const std::string& nameArg) : 
server_(loop,listenAddr,nameArg),
loop_(loop) {
    using namespace std::placeholders;

    //设置新连接创建和断开的回调函数
    server_.setConnectionCallback(std::bind(&ChatServer::OnConnection, this, _1));
    
    //设置用户读写事件的回调函数
    server_.setMessageCallback(std::bind(&ChatServer::OnMessage, this, _1, _2, _3));
    
    //设置线程数量，1个IO线程，3个work线程
    server_.setThreadNum(4);
}

//启动服务器
void ChatServer::Start() {
    server_.start();
}

//用户连接处理回调函数
void ChatServer::OnConnection(const muduo::net::TcpConnectionPtr& conn) {
    if(!conn->connected()) {
        //处理异常退出
        ChatService::Instance()->ClientCloseException(conn);
        conn->shutdown();
    }
}

//用户读写事件处理回调函数
void ChatServer::OnMessage(const muduo::net::TcpConnectionPtr& conn,    //连接
                     muduo::net::Buffer* buffer,            //缓冲区
                     muduo::Timestamp time) {               //时间信息
    std::string buf = buffer->retrieveAllAsString();
    nlohmann::json js = nlohmann::json::parse(buf);         //json反序列化

    //网络层代码和业务层代码进行解耦
    //通过js["msgid"]获取  =》 业务handler  =》  conn js time
    auto msgHandler = ChatService::Instance()->GetHandler(js["msgid"].get<int>());
    msgHandler(conn,js,time);
}          
