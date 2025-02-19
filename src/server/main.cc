#include <signal.h>

#include <iostream>

#include "chat_server.h"
#include "chat_service.h"

void resetHandler(int) {
    ChatService::Instance()->Reset();
    exit(0);
}

int main(int argc, char** argv) {

    if (argc < 3) {
        std::cerr << "commend invalid! example: ./ChatClient 127.0.0.1 6000" << std::endl;
        exit(-1);
    }

    ::signal(SIGINT, resetHandler);

    int port = atoi(argv[2]);

    muduo::net::EventLoop loop;
    muduo::net::InetAddress addr(argv[1],port);
    ChatServer server(&loop,addr,"CharServer");

    server.Start();
    loop.loop();

    return 0;
}