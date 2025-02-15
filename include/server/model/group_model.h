#ifndef GROUP_MODEL_H_
#define GROUP_MODEL_H_

#include <string>
#include <vector>

#include "entity/group.h"

class GroupModel {
public:
    //创建群组
    bool CreateGroup(Group& group);

    //加入群组
    void AddGroup(int userid, int groupid, std::string role);

    //查看群组信息
    std::vector<Group> QueryGroups(int userid);

    //查询群用户
    std::vector<int> QueryGroupUsers(int userid, int groupid);

    //查询指定id群信息
    std::pair<std::vector<GroupUser>,std::vector<std::string>> QueryGroupById(int groupid);
};


#endif