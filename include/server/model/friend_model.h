#ifndef FRIEND_MODEL_H_
#define FRIEND_MODEL_H_

#include <vector>

#include "entity/user.h"

class FriendModel{
public:
    //添加好友关系
    void Insert(int userid, int friendid);
    //返回用户好友列表
    std::vector<User> Query(int userid);
};

#endif