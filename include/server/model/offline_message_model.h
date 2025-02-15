#ifndef OFFLINE_MESSAGE_MODEL_H_ 
#define OFFLINE_MESSAGE_MODEL_H_ 

#include <string>
#include <vector>

class OfflineMsgModel {
public:
    //存储用户的离线消息
    void Insert(int usrid, std::string msg);
    
    //删除用户的离线消息
    void Remove(int usrid);

    //查询用户的离线消息
    std::vector<std::string> Query(int usrid);
};

#endif