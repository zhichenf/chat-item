#ifndef USER_MODEL_H_
#define USER_MODEL_H_

#include "user.h"

class UserModel {
public:
    //插入用户
    bool Insert(User& user);

    //根据id查询用户
    User QueryId(int id);

    //根据姓名查询用户
    User QueryName(std::string name);

    //更新用户信息
    bool UpdateState(User& user);

    //重置用户的状态信息
    void ResetState();

};

#endif