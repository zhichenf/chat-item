#include "chat_server.h"

#include <functional>

// class ChatServer {
// public:
//     ChatServer(muduo::net::EventLoop* loop,                 //事件循环
//             const muduo::net::InetAddress& listenAddr,      //绑定ip地址和端口号
//             const std::string& nameArg);

//     void Start();

// private:
//     //专门处理用户的连接创建
//     auto OnConnection(const muduo::net::TcpConnectionPtr& conn) -> void;

//     //读写事件的回调函数
//     auto OnMessage(const muduo::net::TcpConnectionPtr& conn,    //连接
//                     muduo::net::Buffer* buffer,                //缓冲区
//                     muduo::Timestamp time) -> void;           //时间信息


//     muduo::net::TcpServer server_;      //服务器
//     muduo::net::EventLoop* loop_;       //事件循环指针
// };

//初始化聊天服务器
ChatServer::ChatServer(muduo::net::EventLoop* loop,
             const muduo::net::InetAddress& listenAddr,
             const std::string& nameArg) : 
server_(loop,listenAddr,nameArg),
loop_(loop) {
    using namespace std::placeholders;

    //设置新连接创建的回调函数
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

}

//用户读写事件处理回调函数
void ChatServer::OnMessage(const muduo::net::TcpConnectionPtr& conn,    //连接
                     muduo::net::Buffer* buffer,            //缓冲区
                     muduo::Timestamp time) {               //时间信息


}          



