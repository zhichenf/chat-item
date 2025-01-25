#include "chat_service.h"

#include <muduo/base/Logging.h>

#include "public.h"

//获取单例对象的接口函数
ChatService* ChatService::Instance() {
    static ChatService service;
    return &service;
}

//注册消息以及对应的回调操作
ChatService::ChatService() {
    using namespace std::placeholders;
    msg_handler_map_.insert({kLoginMsg,std::bind(&ChatService::Login, this, _1, _2, _3)});
    msg_handler_map_.insert({kRegMsg,std::bind(&ChatService::Reg, this, _1, _2, _3)});
    msg_handler_map_.insert({kOneChatMsg,std::bind(&ChatService::OneChat, this, _1, _2, _3)});
}

MsgHandler ChatService::GetHandler(int msgid){
    auto it = msg_handler_map_.find(msgid);
    if (msg_handler_map_.end() == it) {
        return [=](const muduo::net::TcpConnectionPtr&, nlohmann::json&,muduo::Timestamp) {
            LOG_ERROR << "msgid:" << msgid << " can not find handler!";
        };
    }
    return msg_handler_map_[msgid];
}

void ChatService::Login(const muduo::net::TcpConnectionPtr& conn, nlohmann::json& js, muduo::Timestamp time) {
    LOG_INFO << "do login service!!";
    //int id = js["id"];      //按id查询
    std::string name = js["name"];        //按姓名查询
    std::string pwd = js["password"];

    User user = user_model_.QueryName(name);
    if (user.GetId() != -1 && user.GetPwd() == pwd) {
        if(user.GetState() == "online") {
            //用户已经登录，不允许重复登录
            nlohmann::json response;
            response["msgid"] = kLoginMsgAck;
            response["errno"] = 2;
            response["errmsg"] = "账号已经登录！";
            conn->send(response.dump());
        } else {
            //登录成功
            user.SetState("online");
            user_model_.UpdateState(user);
            {
                std::lock_guard<std::mutex> lg(mutex_);
                user_conn_map_.insert({user.GetId(),conn});
            }
            

            nlohmann::json response;
            response["msgid"] = kLoginMsgAck;
            response["errno"] = 0;
            response["id"] = user.GetId();
            response["name"] = user.GetName();
            conn->send(response.dump());
        }
    } else {
        //登录失败
        nlohmann::json response;
        response["msgid"] = kLoginMsgAck;
        response["errno"] = 1;
        response["errmsg"] = "用户名或者密码错误！";
        conn->send(response.dump());
    }
}

void ChatService::Reg(const muduo::net::TcpConnectionPtr& conn, nlohmann::json& js, muduo::Timestamp time) {
    LOG_INFO << "do reg service!!";

    std::string name = js["name"];
    std::string pwd = js["password"];
    User user;
    user.SetName(name);
    user.SetPwd(pwd);
    bool state = user_model_.Insert(user);
    if (state) {
        //注册成功
        nlohmann::json response;
        response["msgid"] = kRegMsgAck;
        response["errno"] = 0;
        response["id"] = user.GetId();
        conn->send(response.dump());
    } else {
        //注册失败
        nlohmann::json response;
        response["msgid"] = kRegMsgAck;
        response["errno"] = 1;
        conn->send(response.dump());
    }
}

//一对一聊天业务
void ChatService::OneChat(const muduo::net::TcpConnectionPtr& conn, nlohmann::json& js, muduo::Timestamp time){
    int to_id = js["to"].get<int>();
    bool user_online = false;
    {
        std::lock_guard<std::mutex> lg(mutex_);
        auto it = user_conn_map_.find(to_id);
        if (it != user_conn_map_.end()) {
            //发送给的用户在线
            it->second->send(js.dump());
            return;
        }
    }

    //发送给的用户不在线

}

//处理客户端异常退出
void ChatService::ClientCloseException(const muduo::net::TcpConnectionPtr conn) {
    User user;
    {
        std::lock_guard<std::mutex> lg(mutex_);
        for (auto it = user_conn_map_.begin(), end = user_conn_map_.end(); it != end; ++it) {
            if (it->second == conn) {
                user.SetId(it->first);
                user_conn_map_.erase(it);
                break;
            }
        }
    }

    //更新用户状态
    if (user.GetId() != -1) {
        user.SetState("offline");
        user_model_.UpdateState(user);
    }
}




