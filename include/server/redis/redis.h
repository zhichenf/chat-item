#ifndef REDIS_H_
#define REDIS_H_

#include <hiredis/hiredis.h>
#include <thread>
#include <functional>

class Redis
{
public:
    Redis();
    ~Redis();

    // 连接redis服务器 
    bool Connect();

    // 向redis指定的通道channel发布消息
    bool Publish(int channel, std::string message);

    // 向redis指定的通道subscribe订阅消息
    bool Subscribe(int channel);

    // 向redis指定的通道unsubscribe取消订阅消息
    bool Unsubscribe(int channel);

    // 在独立线程中接收订阅通道中的消息
    void ObserverChannelMessage();

    // 初始化向业务层上报通道消息的回调对象
    void InitNotifyHandler(std::function<void(int, std::string)> fn);

private:
    // hiredis同步上下文对象，负责publish消息
    redisContext* publish_context_;

    // hiredis同步上下文对象，负责subscribe消息
    redisContext* subcribe_context_;

    // 回调操作，收到订阅的消息，给service层上报
    std::function<void(int, std::string)> notify_message_handler_;
};

#endif