#include <signal.h>

#include <iostream>

#include "chat_server.h"
#include "chat_service.h"

void resetHandler(int) {
    ChatService::Instance()->Reset();
    exit(0);
}

int main() {

    ::signal(SIGINT, resetHandler);

    muduo::net::EventLoop loop;
    muduo::net::InetAddress addr("127.0.0.1",6000);
    ChatServer server(&loop,addr,"CharServer");

    server.Start();
    loop.loop();

    return 0;
}