#ifndef GROUP_H_
#define GROUP_H_

#include <string>
#include <vector>

#include "entity/group_user.h"

class Group{
public:
    Group(int id = -1, std::string name = "", std::string desc = "");
    
    void SetId(int id);
    void SetName(std::string name);
    void SetDesc(std::string desc);

    int GetId();
    std::string GetName();
    std::string GetDesc();
    std::vector<GroupUser>& GetUsers();

private:
    int id_;
    std::string name_;
    std::string desc_;
    std::vector<GroupUser> users_;
};

#endif