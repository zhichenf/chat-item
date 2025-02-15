#ifndef GROUP_USER_H_
#define GROUP_USER_H_

#include "entity/user.h"

class GroupUser : public User{
public:
    void SetRole(std::string role);
    std::string GetRole();
private:
    std::string role_;
};

#endif