#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <iostream>
#include <functional>
#include <string>

using namespace std;
using namespace muduo;
using namespace muduo::net;

/*
    基于muduo网络库开发服务器程序
    1.组合TCPserver对象
    2.创建EventLoop事件循环对象的指针
    3.明确TcpServer构造函数
*/
class ChatServer {
public:
    ChatServer(EventLoop* loop,                 //事件循环
            const InetAddress& listenAddr,      //绑定ip地址和端口号
            const string& nameArg) :            //服务器的名称
            server_(loop,listenAddr,nameArg),
            loop_(loop) {
                using namespace placeholders;
                //给服务器注册用户链接的创建和断开回调
                server_.setConnectionCallback(std::bind(&ChatServer::OnConnection, this, _1));
            
                //给服务器注册用户读写事件回调
                server_.setMessageCallback(std::bind(&ChatServer::OnMessage, this, _1, _2, _3));

                //设置服务器端的线程数量, 1个IO线程，3个work线程
                server_.setThreadNum(4);
            }

    void Start() {
        server_.start();
    }

private:

    //专门处理用户的连接创建和断开
    void OnConnection(const TcpConnectionPtr& conn) {
        if (conn->connected()) {
            cout << conn->peerAddress().toIpPort() << " -----> " << conn->localAddress().toIpPort() << 
            " state:online " << endl;
        } else {
            cout << " state:offline " << endl;
            conn->shutdown();
        }
        
    }

    void OnMessage(const TcpConnectionPtr& conn,        //连接
                    Buffer* buffer,                //缓冲区
                    Timestamp time) {           //时间信息
        string buf = buffer -> retrieveAllAsString();
        cout << "recv data = " << buf << ", time:" << time.toString() << endl;
        conn->send(buf);
    }


    TcpServer server_;      //服务器
    EventLoop* loop_;       //事件循环指针
};

int main() {
    EventLoop loop;
    InetAddress addr("127.0.0.1", 6000);
    ChatServer server(&loop, addr, "ChatServer");

    server.Start();     //启动服务，添加epoll
    loop.loop();        //epoll_wait以阻塞方式等待新用户连接

    return 0;
}