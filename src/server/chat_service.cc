#include "chat_service.h"

#include <muduo/base/Logging.h>

#include <vector>
#include <string>
#include <iostream>

#include "public.h"
#include "entity/group.h"

#include "json.hpp"

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
    msg_handler_map_.insert({kAddFriendMsg,std::bind(&ChatService::AddFriend, this, _1, _2, _3)});
    msg_handler_map_.insert({kCreateGroupMsg,std::bind(&ChatService::CreateGroup, this, _1, _2, _3)});
    msg_handler_map_.insert({kAddGroupMsg,std::bind(&ChatService::AddGroup, this, _1, _2, _3)});
    msg_handler_map_.insert({kGroupChatMsg,std::bind(&ChatService::GroupChat, this, _1, _2, _3)});
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
    std::cout << "do login service!!" << std::endl;  
    std::string name = js["name"];       
    std::string pwd = js["password"];

    User user = user_model_.QueryName(name);
    if (user.GetId() != -1 && user.GetPwd() == pwd) {
        if(user.GetState() == "online") {
            //用户已经登录，不允许重复登录
            nlohmann::json response;
            response["msgid"] = kLoginMsgAck;
            response["errno"] = 2;
            response["errmsg"] = "user id already login";
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
            
            //检查是否有离线消息，如果有，则要一起发送回去
            std::vector<std::string> vec = offline_msg_model_.Query(user.GetId());
            if (!vec.empty()) {
                response["offlinemsg"] = vec;
                offline_msg_model_.Remove(user.GetId());
            }

            //查询该用户的好友信息 
            std::vector<User> friends_vec = friend_model_.Query(user.GetId());
            if (!friends_vec.empty()) {
                std::cout << " you have friend" << std::endl;
                std::vector<std::string> vec2;
                for (auto& user2 : friends_vec) {
                    nlohmann::json js2;
                    js2["id"] = user2.GetId();
                    js2["name"] = user2.GetName();
                    js2["state"] = user2.GetState();
                    vec2.push_back(js2.dump());

                    //通知在线好友用户已经上线
                    {
                        nlohmann::json js3;
                        js3["msgid"] = kNotifyFriend;
                        js3["userid"] = user.GetId();
                        std::lock_guard<std::mutex> lock(mutex_);
                        auto it = user_conn_map_.find(user2.GetId());
                        if (it != user_conn_map_.end()) {
                            it->second->send(js3.dump());
                        }
                    }
                }
                response["friends"] = vec2;
            } 

            //查询用户的群组信息
            std::vector<Group> group_user_vec = group_model_.QueryGroups(user.GetId());

            if (!group_user_vec.empty()) {
                std::cout << " you have group" << std::endl;
                std::vector<std::string> group_v;
                for (auto& group : group_user_vec) {
                    nlohmann::json grpjson;
                    grpjson["id"] = group.GetId();
                    grpjson["groupname"] = group.GetName();
                    grpjson["desc"] = group.GetDesc();
                    std::vector<std::string> user_v;
                    for (auto& user : group.GetUsers()) {
                        nlohmann::json js;
                        js["id"] = user.GetId();
                        js["name"] = user.GetName();
                        js["state"] = user.GetState();
                        js["role"] = user.GetRole();
                        user_v.push_back(js.dump());
                    }
                    grpjson["users"] = user_v;
                    group_v.push_back(grpjson.dump());
                }
                response["groups"] = group_v;
             }

            conn->send(response.dump());
        }
    } else {
        //登录失败
        nlohmann::json response;
        response["msgid"] = kLoginMsgAck;
        response["errno"] = 1;
        response["errmsg"] = "user name or password error！";
        conn->send(response.dump());
    }
}

void ChatService::Reg(const muduo::net::TcpConnectionPtr& conn, nlohmann::json& js, muduo::Timestamp time) {
    std::cout << "do reg service!!" << std::endl;

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
    std::cout << "do onechat service!!" << std::endl;
    int to_id = js["toid"].get<int>();
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
    offline_msg_model_.Insert(to_id,js.dump());
}

//添加好友业务
void ChatService::AddFriend(const muduo::net::TcpConnectionPtr& conn, nlohmann::json& js, muduo::Timestamp time) {
    std::cout << "do addfriend service!!" << std::endl;
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    friend_model_.Insert(userid,friendid);

    //回应自己
    User user = user_model_.QueryId(friendid);
    nlohmann::json res;
    res["msgid"] = kAddFriendMsgAck;
    res["id"] = user.GetId();
    res["name"] = user.GetName();
    res["state"] = user.GetState();
    res["myname"] = js["name"];
    res["myid"] = userid;
    conn->send(res.dump());

    //回应在线好友
    std::lock_guard<std::mutex> lg(mutex_);
    auto it = user_conn_map_.find(friendid);
    if (it != user_conn_map_.end()) {
        it->second->send(res.dump());
    }
}

//创建群组业务
void ChatService::CreateGroup(const muduo::net::TcpConnectionPtr& conn, nlohmann::json& js, muduo::Timestamp time) {
    std::cout << "do creategroup service!!" << std::endl;
    int userid = js["id"].get<int>();
    std::string name = js["groupname"];
    std::string desc = js["desc"];

    Group group(-1,name,desc);
    if (group_model_.CreateGroup(group)) {
        group_model_.AddGroup(userid,group.GetId(),"creator");
    }

    nlohmann::json resp;
    resp["msgid"] = kCreateGroupMsgAck;
    resp["id"] = group.GetId();
    resp["name"] = group.GetName();
    resp["desc"] = group.GetDesc();
    conn->send(resp.dump());
}

//加入群组业务
void ChatService::AddGroup(const muduo::net::TcpConnectionPtr& conn, nlohmann::json& js, muduo::Timestamp time) {
    std::cout << "do addgroup service!!" << std::endl;
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    group_model_.AddGroup(userid,groupid,"normal");
    std::pair<std::vector<GroupUser>,std::vector<std::string>> p =  group_model_.QueryGroupById(groupid);
    std::vector<GroupUser> user_vec = p.first;
    std::vector<std::string> users;
    for (auto& user : user_vec) {
        nlohmann::json js;
        js["id"] = user.GetId();
        js["name"] = user.GetName();
        js["state"] = user.GetState();
        js["role"] = user.GetRole();
        users.push_back(js.dump());
    }
    nlohmann::json resp;

    std::vector<std::string> group_info = p.second;
    resp["msgid"] = kAddGroupMsgAck;
    resp["groupid"] = groupid;
    resp["groupname"] = group_info[0];
    resp["groupdesc"] = group_info[1];
    resp["id"] = userid;
    resp["name"] = js["name"];
    resp["state"] = js["state"];
    resp["friend"] = users;
    
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& user : user_vec) {
        auto it = user_conn_map_.find(user.GetId());
        if (it != user_conn_map_.end()) {
            it->second->send(resp.dump());
        }
    }
}

//群组聊天业务
void ChatService::GroupChat(const muduo::net::TcpConnectionPtr& conn, nlohmann::json& js, muduo::Timestamp time) {
    std::cout << "do groupchat service!!" << std::endl;
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    std::vector<int> userid_vec = group_model_.QueryGroupUsers(userid,groupid);

    std::lock_guard<std::mutex> lock(mutex_);
    for (int id : userid_vec) {
        auto it = user_conn_map_.find(id);
        if (it != user_conn_map_.end()) {
            it->second->send(js.dump());
        } else {
            offline_msg_model_.Insert(id,js.dump());
        }
    }
}

//服务器异常，业务重置
void ChatService::Reset() {
    //把online状态的用户设置为offline
    user_model_.ResetState();
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

     //查询该用户的好友信息 
     std::vector<User> friends_vec = friend_model_.Query(user.GetId());
     if (!friends_vec.empty()) {
         std::vector<std::string> vec2;
         for (auto& user2 : friends_vec) {
             //通知在线好友用户下线
             {
                 nlohmann::json js3;
                 js3["msgid"] = kNotifyFriendExit;
                 js3["userid"] = user.GetId();
                 std::lock_guard<std::mutex> lock(mutex_);
                 auto it = user_conn_map_.find(user2.GetId());
                 if (it != user_conn_map_.end()) {
                     it->second->send(js3.dump());
                 }
             }
         }
     } 

    //更新用户状态
    if (user.GetId() != -1) {
        user.SetState("offline");
        user_model_.UpdateState(user);
    }
}