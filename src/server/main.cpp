#include <iostream>

#include "chat_server.h"

int main() {
    muduo::net::EventLoop loop;
    muduo::net::InetAddress addr("127.0.0.1",6000);
    ChatServer server(&loop,addr,"CharServer");

    server.Start();
    loop.loop();

    return 0;
}