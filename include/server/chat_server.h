#ifndef CHAT_SERVER_H_
#define CHAT_SERVER_H_

#include <string>

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>

//聊天服务器的主类
class ChatServer {
public:
    ChatServer(muduo::net::EventLoop* loop,                 //事件循环
            const muduo::net::InetAddress& listenAddr,      //绑定ip地址和端口号
            const std::string& nameArg);

    void Start();

private:
    //专门处理用户的连接创建
    void OnConnection(const muduo::net::TcpConnectionPtr& conn);

    //读写事件的回调函数
    void OnMessage(const muduo::net::TcpConnectionPtr& conn,    //连接
                    muduo::net::Buffer* buffer,                //缓冲区
                    muduo::Timestamp time);                     //时间信息

    muduo::net::TcpServer server_;      //服务器
    muduo::net::EventLoop* loop_;       //事件循环指针
};

#endif 
