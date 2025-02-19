#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>
#include <thread>
#include <string> 
#include <vector>
#include <chrono>
#include <ctime>
#include <functional>
#include <iomanip>

#include "entity/group.h"
#include "entity/group_user.h"
#include "entity/user.h"
#include "public.h"

#include "json.hpp"

#define DEBUG 0

// 记录当前系统登录的用户信息
User g_current_user;
// 记录当前登录用户的好友列表
std::vector<User> g_current_user_friends_list;
// 记录当前登录用户的群组列表
std::vector<Group> g_current_user_groups_list;
//显示当前登录用户的基本信息
void ShowCurrenUserData();

//接受线程
void ReadTaskHandler(int clientfd);
//获取系统时间
std::string GetCurrentTime();
//主聊天页面程序
void MainMenu(int clientfd);

void ShowCurrentUserData() {
    std::cout << "====================================" << std::endl;
    std::cout << "user name: " << g_current_user.GetName() << "(" << g_current_user.GetId() << ")" <<std::endl;
    std::cout << "------------------------------------" << std::endl;
    std::cout << "user friends:" << std::endl;
    if (!g_current_user_friends_list.empty()) {
        for (auto& f : g_current_user_friends_list) {
            std::cout << "      " << f.GetName() << " (" << f.GetState() << ")" << std::endl;
        }
    } else {
        std::cout << "      " << "None" << std::endl;
    }

    std::cout << "------------------------------------" << std::endl;
    std::cout << "user groups:" << std::endl;
    if (!g_current_user_groups_list.empty()) {
        for (auto& g : g_current_user_groups_list) {
            std::cout << "      " << g.GetName() << "(" << g.GetId() << ") " << g.GetDesc() << std::endl;
        }   
    } else {
        std::cout << "      " << "None" << std::endl;
    }

    std::cout << "====================================" << std::endl;
}

//接受线程
void ReadTaskHandler(int clientfd) {
    while(1) {
        char buffer[1024] = {0};
        int len = recv(clientfd,buffer,1024,0);
        if (-1 == len || 0 == len) {
            close(clientfd);
            exit(-1);
        }

        //接受服务器转发数据
        nlohmann::json js = nlohmann::json::parse(buffer);
        if (kOneChatMsg == js["msgid"].get<int>()) {        //发送的消息
            std::cout << js["time"].get<std::string>() << " [" << js["id"] << "]" << js["name"].get<std::string>()
            << " said: " << js["msg"].get<std::string>() << std::endl;
        } else if (kAddFriendMsgAck == js["msgid"].get<int>()) {        //添加好友回应
            if (g_current_user.GetId() == js["myid"].get<int>()) {
                User user;
                user.SetId(js["id"].get<int>());
                user.SetName(js["name"]);
                user.SetState(js["state"]);
                g_current_user_friends_list.push_back(user);
            } else {
                User user;
                user.SetId(js["myid"].get<int>());
                user.SetName(js["myname"]);
                user.SetState("online");
                g_current_user_friends_list.push_back(user);
            }

        } else if (kCreateGroupMsgAck == js["msgid"].get<int>()) {      //创建群聊回应
            Group group;
            group.SetId(js["id"].get<int>());
            group.SetName(js["name"]);
            group.SetDesc(js["desc"]);
            GroupUser user;
            user.SetId(g_current_user.GetId());
            user.SetName(g_current_user.GetName());
            user.SetState(g_current_user.GetState());
            user.SetRole("creator");
            group.GetUsers().push_back(user);
            g_current_user_groups_list.push_back(group);
        } else if (kAddGroupMsgAck == js["msgid"].get<int>()) {         //添加群组回应
            std::cout << g_current_user.GetId() << " add new group member" << std::endl;
            if (js["id"] == g_current_user.GetId()) {               //如果我是新加入的用户
                Group group;
                group.SetId(js["groupid"].get<int>());
                group.SetName(js["groupname"]);
                group.SetDesc(js["groupdesc"]);
                std::vector<std::string> users = js["friend"].get<std::vector<std::string>>();
                for (auto& user_str : users) {
                    nlohmann::json user_json = nlohmann::json::parse(user_str);
                    GroupUser user;
                    user.SetId(user_json["id"].get<int>());
                    user.SetName(user_json["name"]);
                    user.SetState(user_json["state"]);
                    user.SetRole(user_json["role"]);
                    group.GetUsers().push_back(user);
                }
                g_current_user_groups_list.push_back(group);
            } else {                                                //如果我不是新加入的用户
                for (auto& group : g_current_user_groups_list) {
                    if (group.GetId() == js["groupid"].get<int>()) {
                        GroupUser user;
                        user.SetId(js["id"].get<int>());
                        user.SetName(js["name"]);
                        user.SetState(js["state"]);
                        user.SetRole("normal");
                        group.GetUsers().push_back(user);
                        break;
                    }
                }
            }
        } else if (kNotifyFriend == js["msgid"].get<int>()) {       //如果是有好友登录
            std::cout << " you have a friend login" << std::endl;
            for (auto& user : g_current_user_friends_list) {
                if (user.GetId() == js["userid"].get<int>()) {
                    user.SetState("online");
                }
            }
        } else if (kNotifyFriendExit == js["msgid"].get<int>()) {
            for (auto& user : g_current_user_friends_list) {
                if (user.GetId() == js["userid"].get<int>()) {
                    user.SetState("offline");
                }
            }
        } else if (kNotifyGroup == js["msgid"].get<int>()) {        //如果是群成员登录
            for (auto& group : g_current_user_groups_list) {
                if (group.GetId() == js["groupid"].get<int>()) {
                    for (auto& user : group.GetUsers()) {
                        if (user.GetId() == js["userid"].get<int>()) {
                            user.SetState("online");
                        }
                    }
                }
            }
        } else if (kNotifyGroupExit == js["msgid"].get<int>()) {
            for (auto& group : g_current_user_groups_list) {
                if (group.GetId() == js["groupid"].get<int>()) {
                    for (auto& user : group.GetUsers()) {
                        if (user.GetId() == js["userid"].get<int>()) {
                            user.SetState("offline");
                        }
                    }
                }
            }
        }  else if (kGroupChatMsg == js["msgid"].get<int>()) {            //收到群消息
            std::string groupname = "";
            for (auto& group : g_current_user_groups_list) {
                if (group.GetId() == js["togroup"].get<int>()) {
                    groupname = group.GetName();
                }
            }
            std::cout << js["time"].get<std::string>() << " [" << js["id"] << "]" << js["name"].get<std::string>()
            << " in group " << groupname << " said: " << js["msg"].get<std::string>() << std::endl; 
        }
    }
}


std::string GetCurrentTime() {
    // 获取当前时间
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);

    // 使用字符串流格式化时间
    std::ostringstream oss;
    oss << std::put_time(localTime, "%Y-%m-%d %H:%M");

    // 返回格式化后的时间字符串
    return oss.str();
}

int main(int argc, char** argv) {


#if DEBUG
    const char* ip = "127.0.0.1";
    uint16_t port = 6000;
#else
    char* ip = argv[1];
    uint16_t port = atoi(argv[2]);
    if (argc < 3) {
        std::cerr << "commend invalid! example: ./ChatClient 127.0.0.1 6000" << std::endl;
        exit(-1);
    }
#endif

    //创建socket
    int clientfd = socket(AF_INET,SOCK_STREAM, 0);
    if (-1 == clientfd) {
        std::cerr << "socket create error" << std::endl;
        exit(-1);
    }

    sockaddr_in server;
    memset(&server,0,sizeof(server));

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    if (-1 == connect(clientfd, (sockaddr*)&server, sizeof(sockaddr_in))) {
        std::cerr << " connect serve error" << std::endl;
        close(clientfd);
        exit(-1);
    }

    while(1) {
        std::cout << "=================================" << std::endl;
        std::cout << "1. login" << std::endl;
        std::cout << "2. register" << std::endl;
        std::cout << "3. exit" << std::endl;
        std::cout << "=================================" << std::endl;
        std::cout << "choice: ";
        std::string str;
        getline(std::cin,str);

        if (str.size() != 1 || str[0] < '1' || str[0] > '3') {
            std::cout << "invalid input" << std::endl;
            continue;
        } 
        int choice = atoi(str.c_str());
        switch(choice) {
            case 1:
                {   //登录业务
                    std::cout << "please input your account name :";
                    std::string name;
                    std::getline(std::cin, name);
                    std::cout << "please input your password:";
                    std::string password;
                    std::getline(std::cin, password);

                    nlohmann::json js;
                    js["msgid"] = kLoginMsg;
                    js["id"] = g_current_user.GetId();
                    js["name"] = name;
                    js["password"] = password;
                    std::string request = js.dump();

                    int len = send(clientfd, request.c_str(), request.size()+1, 0);
                    if (len == -1) {
                        std::cerr << "send login message error" << std::endl;
                    } else {
                        char buff[1024] = {0};
                        len = recv(clientfd,buff,sizeof(buff),0);
                        if (-1 == len) {
                            std::cerr << "receive message error" << std::endl;
                        } else {
                            nlohmann::json js = nlohmann::json::parse(buff);
                            if (1 == js["errno"].get<int>()) {
                                std::cout << "login error, user name or password error!!!" << std::endl;
                            } else if (2 == js["errno"].get<int>()) {
                                std::cout << "user is already login!!!" << std::endl;
                            } else {
                                g_current_user.SetId(js["id"].get<int>());
                                g_current_user.SetName(name);
                                g_current_user.SetState("online");

                                //记录用户好友信息
                                if (js.contains("friends")) {
                                    std::vector<std::string> vec = js["friends"].get<std::vector<std::string>>();
                                    for (auto& str : vec) {
                                        nlohmann::json js = nlohmann::json::parse(str);
                                        User user;
                                        user.SetId(js["id"].get<int>());
                                        user.SetName(js["name"]);
                                        user.SetState(js["state"]);
                                        g_current_user_friends_list.push_back(user);
                                    }
                                } 

                                //记录用户群组信息
                                if (js.contains("groups")) {
                                    std::vector<std::string> vec = js["groups"].get<std::vector<std::string>>();
                                    for (auto& str : vec) {
                                        nlohmann::json js = nlohmann::json::parse(str);
                                        Group group;
                                        group.SetId(js["id"].get<int>());
                                        group.SetName(js["groupname"]);
                                        group.SetDesc(js["desc"]);

                                        std::vector<std::string> vec2 = js["users"].get<std::vector<std::string>>();
                                        for (auto& userstr : vec2) {
                                            GroupUser user;
                                            nlohmann::json js2 = nlohmann::json::parse(userstr);
                                            user.SetId(js2["id"].get<int>());
                                            user.SetName(js2["name"]);
                                            user.SetState(js2["state"]);
                                            user.SetRole(js2["role"]);
                                            group.GetUsers().push_back(user);
                                        }
                                        g_current_user_groups_list.push_back(group);
                                    }
                                }
                                ShowCurrentUserData();

                                if (js.contains("offlinemsg")) {
                                    std::vector<std::string> vec = js["offlinemsg"].get<std::vector<std::string>>();
                                    for (auto& std : vec) {
                                        nlohmann::json js2 = nlohmann::json::parse(std);
                                        if (js2["msgid"] == kGroupChatMsg) {
                                            std::string groupname = "";
                                            for (auto& group : g_current_user_groups_list) {
                                                if (group.GetId() == js2["togroup"].get<int>()) {
                                                    groupname = group.GetName();
                                                }
                                            }
                                            std::cout << js2["time"].get<std::string>() << " [" << js2["id"] << "]" << js2["name"].get<std::string>()
                                            << " in group " << groupname << " said: " << js2["msg"].get<std::string>() << std::endl; 
                                        } else {
                                            std::cout << js2["time"] << "[" << js2["id"] << "]" << js2["name"] 
                                            << " said: " << js2["msg"] << std::endl;
                                        }
                                    }
                                }

                                std::thread readTask(ReadTaskHandler, clientfd);
                                readTask.detach();

                                MainMenu(clientfd);
                            }
                        }
                    }
                }
                break;
            case 2:
                {   //注册业务
                    std::cout << "please input your account name :";
                    std::string name;
                    std::getline(std::cin, name);
                    std::cout << "please input your password:";
                    std::string password;
                    std::getline(std::cin,password);

                    nlohmann::json js;
                    js["msgid"] = kRegMsg;
                    js["name"] = name;
                    js["password"] = password;
                    std::string request = js.dump();

                    int len = send(clientfd, request.c_str(), request.size()+1, 0);
                    if (len == -1) {
                        std::cerr << "send register message error" << std::endl;
                    } else {
                        char buff[1024] = {0};
                        len = recv(clientfd,buff,sizeof(buff),0);
                        if (-1 == len) {
                            std::cerr << "receive message error" << std::endl;
                        } else {
                            nlohmann::json js = nlohmann::json::parse(buff);
                            if (1 == js["errno"].get<int>()) {
                                std::cout << "register error " << std::endl;
                            } else {
                                std::cout << "succeed register" << std::endl;
                            }
                        }
                    }
                }
                break;
            case 3:
                {       //退出
                    close(clientfd);
                    exit(0);
                }
                break;
            default:    //默认
                {
                    std::cerr << "invalid input" << std::endl;
                }
                break;
        }
    }

}

// "help" command handler
void Help(int fd = 0, std::string str = "");
// "info" command handler
void Info(int fd = 0, std::string str = "");
// "groupmember" command handler
void GroupMember(int, std::string);
// "chat" command handler
void Chat(int, std::string);
// "addfriend" command handler
void AddFriend(int, std::string);
// "creategroup" command handler
void CreateGroup(int, std::string);
// "addgroup" command handler
void AddGroup(int, std::string);
// "groupchat" command handler
void GroupChat(int, std::string);
// "quit" command handler
void Quit(int,std::string);


//系统支持的客户端命令列表
std::unordered_map<std::string,std::string> command_map = {
    {"'help'", "show all the commands."},
    {"'info", "show the user's information"},
    {"'groupmember:groupid", "show the member of group"},
    {"'chat:friendid:message'", "chat to your friend."},
    {"'addfriend:friendid'", "add friend."},
    {"'creategroup:groupname:groupdesc'", "create group."},
    {"'addgroup:groupid'", "add group."},
    {"'groupchat:groupid:message'", "chat in a group."},
    {"'quit'", "close the program."}
};

std::unordered_map<std::string,std::function<void(int,std::string)>> command_handler_map = {
    {"help", Help},
    {"info", Info},
    {"groupmember",GroupMember},
    {"chat", Chat},
    {"addfriend", AddFriend},
    {"creategroup", CreateGroup},
    {"addgroup", AddGroup},
    {"groupchat", GroupChat},
    {"quit", Quit}
};

void MainMenu(int clientfd) {
    std::string command;
    while(1) {
        std::cout << "please input command, input \"help\" can show the all commands." << std::endl;
        getline(std::cin,command);
        int index = command.find(":");
        std::string command_base = "";
        std::string command_info = "";
        if (index == -1) {
            command_base = command;
        } else {
            command_base = command.substr(0,index);
            command_info = command.substr(index + 1);
        }
        auto it = command_handler_map.find(command_base);
        if (it == command_handler_map.end()) {
            std::cout << "you have entered wrong command" << std::endl;
        } else {
            (it->second)(clientfd, command_info);
        }
    }
}

// "help" command handler
void Help(int fd, std::string str) {
    std::cout << "do help service!!" << std::endl;
    for (auto& p : command_map) {
        std::cout << p.first << " :         " << p.second << std::endl;
    }
}

// "info" command handler
void Info(int clientfd, std::string) {
    ShowCurrentUserData();
}

// "groupmember" command handler
void GroupMember(int fd, std::string command_info) {
    std::cout << "do groupmember service!!" << std::endl;
    int groupid = atoi(command_info.c_str());
    for (auto& group : g_current_user_groups_list) {
        if (group.GetId() == groupid) {
            std::cout << "----------------" << group.GetName() <<  "----------------" << std::endl;
            for (auto& user : group.GetUsers()) {
                std::cout << user.GetName() << "--->" << user.GetRole() << "(" << user.GetState() << ")" << std::endl;
            }
            std::cout << "----------------" << group.GetName() <<  "----------------" << std::endl;
        }
    }
}

// "chat" command handler
void Chat(int clientfd, std::string command_info) {         
    std::cout << "do chat service!!" << std::endl;
    int index = command_info.find(":");
    int friendid = atoi(command_info.substr(0,index).c_str());
    std::string message = command_info.substr(index+1);

    nlohmann::json js;
    js["msgid"] = kOneChatMsg;
    js["id"] = g_current_user.GetId();
    js["name"] = g_current_user.GetName();
    js["toid"] = friendid;
    js["msg"] = message;
    js["time"] = GetCurrentTime();

    std::string buf = js.dump();

    int len = send(clientfd,buf.c_str(),buf.size() + 1,0);
    if (-1 == len)
    {
        std::cerr << "send error -> " << buf << std::endl;
    }
}

// "addfriend" command handler
void AddFriend(int clientfd, std::string command_info) {
    std::cout << " do addfriend service" << std::endl;
    int friendid = atoi(command_info.c_str());

    for (auto& user : g_current_user_friends_list) {
        if (friendid == user.GetId()) {
            std::cout << " you have added this friend" << std::endl;
            return;
        }
    }

    nlohmann::json js;
    js["msgid"] = kAddFriendMsg;
    js["id"] = g_current_user.GetId();
    js["name"] = g_current_user.GetName();
    js["friendid"] = friendid;

    std::string buf = js.dump();

    int len = send(clientfd,buf.c_str(),buf.size() + 1,0);
    if (-1 == len)
    {
        std::cerr << "send  error -> " << buf << std::endl;
    }
}

// "creategroup" command handler
void CreateGroup(int clientfd, std::string command_info) {
    int index = command_info.find(":");
    std::string groupname = command_info.substr(0,index);
    std::string groupdesc = command_info.substr(index+1);

    nlohmann::json js;
    js["msgid"] = kCreateGroupMsg;
    js["id"] = g_current_user.GetId();
    js["groupname"] = groupname;
    js["desc"] = groupdesc;

    std::string buf = js.dump();

    int len = send(clientfd,buf.c_str(),buf.size() + 1,0);
    if (-1 == len)
    {
        std::cerr << "send error -> " << buf << std::endl;
    }
}

// "addgroup" command handler
void AddGroup(int clientfd, std::string command_info) {
    int groupid = atoi(command_info.c_str());
    nlohmann::json js;
    js["msgid"] = kAddGroupMsg;
    js["id"] = g_current_user.GetId();
    js["groupid"] = groupid;
    js["name"] = g_current_user.GetName();
    js["state"] = g_current_user.GetState();

    std::string buf = js.dump();

    int len = send(clientfd,buf.c_str(),buf.size() + 1,0);
    if (-1 == len)
    {
        std::cerr << "send error -> " << buf << std::endl;
    }
}

// "groupchat" command handler
void GroupChat(int clientfd, std::string command_info) {
    std::cout << "do groupchat service!!" << std::endl;
    int index = command_info.find(":");
    int groupid = atoi(command_info.substr(0,index).c_str());
    std::string message = command_info.substr(index+1);

    nlohmann::json js;
    js["msgid"] = kGroupChatMsg;
    js["id"] = g_current_user.GetId();
    js["name"] = g_current_user.GetName();
    js["togroup"] = groupid;
    js["msg"] = message;
    js["time"] = GetCurrentTime();

    std::string buf = js.dump();
    int len = send(clientfd, buf.c_str(), buf.size() + 1, 0);
    if (-1 == len) {
        std::cerr << "send error -> " << buf << std::endl;
    }
}

// "quit" command handler
void Quit(int clientfd, std::string command_info) {
    nlohmann::json js;
    js["msgid"] = kQuitMsg;
    js["id"] = g_current_user.GetId();

    std::string buf = js.dump();
    int len = send(clientfd, buf.c_str(), buf.size() + 1, 0);
    if (-1 == len) {
        std::cerr << "send error -> " << buf << std::endl;
    }

    exit(0);
}