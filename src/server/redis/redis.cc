#include "redis/redis.h"

#include <iostream>

Redis::Redis() : publish_context_(nullptr), subcribe_context_(nullptr) {

}

Redis::~Redis() {
    if (publish_context_) {
        redisFree(publish_context_);
    }
    if (subcribe_context_) {
        redisFree(subcribe_context_);
    }
}

// 连接redis服务器 
bool Redis::Connect() {
    publish_context_ = redisConnect("127.0.0.1", 6379);
    if (!publish_context_) {
        std::cerr << "connect redis failed" << std::endl;
        return false;
    }

    subcribe_context_ = redisConnect("127.0.0.1", 6379);
    if (!subcribe_context_) {
        std::cerr << "connect redis failed" << std::endl;
        return false;
    }

    std::thread t([&]() {
        ObserverChannelMessage();
    });
    t.detach();

    std::cout << "connect redis server success!" << std::endl;

    return true;
}

// 向redis指定的通道channel发布消息
bool Redis::Publish(int channel, std::string message) {
    std::unique_ptr<redisReply, decltype(&freeReplyObject)> reply(
        static_cast<redisReply*>(redisCommand(publish_context_, "publish %d %s",channel, message.c_str())), freeReplyObject);

    if (!reply) {
        std::cerr << "publish command failed!" << std::endl;
        return false;
    }
    return true;
}

// 向redis指定的通道subscribe订阅消息
bool Redis::Subscribe(int channel) {
    if (REDIS_ERR == redisAppendCommand(subcribe_context_, "subscribe %d", channel)) {
        std::cerr << "subscribe command failed!" << std::endl;
        return false;
    }

    int done = 0;
    while(!done) {
        if (REDIS_ERR == redisBufferWrite(subcribe_context_, &done)) {
            std::cerr << " subscribe command failed!" << std::endl;
            return false;
        }
    }
    return true;
}

// 向redis指定的通道unsubscribe取消订阅消息
bool Redis::Unsubscribe(int channel) {
    if (REDIS_ERR == redisAppendCommand(subcribe_context_, "unsubscribe %d", channel)) {
        std::cerr << "unsubscribe command failed!" << std::endl;
        return false;
    }

    int done = 0;
    while(!done) {
        if (REDIS_ERR == redisBufferWrite(subcribe_context_, &done)) {
            std::cerr << "unsubscribe command failed!" << std::endl;
            return false;
        }
    }
    return true;
}

// 在独立线程中接收订阅通道中的消息
void Redis::ObserverChannelMessage() {
    redisReply *reply_ptr = nullptr;
    while (REDIS_OK == redisGetReply(subcribe_context_, (void**)&reply_ptr)) {  //该函数会发生阻塞
        std::unique_ptr<redisReply, decltype(&freeReplyObject)> reply(reply_ptr, freeReplyObject);
        if (reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr) {
            notify_message_handler_(atoi(reply->element[1]->str), reply->element[2]->str);
        }
    }
}

// 初始化向业务层上报通道消息的回调对象
void Redis::InitNotifyHandler(std::function<void(int, std::string)> fn) {
    notify_message_handler_ = fn;
}