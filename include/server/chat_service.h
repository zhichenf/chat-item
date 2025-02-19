#ifndef CHAT_SERVICE_H_
#define CHAT_SERVICE_H_

#include <unordered_map>
#include <functional>
#include <mutex>

#include <muduo/net/TcpConnection.h>
#include "json.hpp"

#include "model/user_model.h"
#include "model/offline_message_model.h"
#include "model/friend_model.h"
#include "model/group_model.h"
#include "redis/redis.h"

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
    //添加好友业务
    void AddFriend(const muduo::net::TcpConnectionPtr& conn, nlohmann::json& js, muduo::Timestamp time);
    //创建群组业务
    void CreateGroup(const muduo::net::TcpConnectionPtr& conn, nlohmann::json& js, muduo::Timestamp time);
    //加入群组业务
    void AddGroup(const muduo::net::TcpConnectionPtr& conn, nlohmann::json& js, muduo::Timestamp time);
    //群组聊天业务
    void GroupChat(const muduo::net::TcpConnectionPtr& conn, nlohmann::json& js, muduo::Timestamp time);
    //请求最新信息业务
    void Info(const muduo::net::TcpConnectionPtr& conn, nlohmann::json& js, muduo::Timestamp time);
    //退出业务
    void Quit(const muduo::net::TcpConnectionPtr& conn, nlohmann::json& js, muduo::Timestamp time);

    //redis上的消息回调
    void HandleRedisSubscribeMessage(int , std::string);
    
    //获取消息对应的处理器
    MsgHandler GetHandler(int msgid);
    //服务器异常，业务重置
    void Reset();
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
    OfflineMsgModel offline_msg_model_;
    FriendModel friend_model_;
    GroupModel group_model_;

    Redis redis_;
};


#endif