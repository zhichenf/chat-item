#include "model/group_model.h"

#include <string.h>

#include <memory>

#include "db/db.h"

//创建群组
bool GroupModel::CreateGroup(Group& group) {
    char sql[1024] = {0};
    sprintf(sql, "insert into AllGroup(groupname, groupdesc) values ('%s', '%s');", 
    group.GetName().c_str(), group.GetDesc().c_str());

    MySQL mysql;
    if (mysql.Connect()) {
        if (mysql.Update(sql)) {
            group.SetId(mysql_insert_id(mysql.GetConnection()));
            return true;
        }
    }

    return false;
}

//加入群组
void GroupModel::AddGroup(int userid, int groupid, std::string role) {
    char sql[1024] = {0};
    sprintf(sql, "insert into GroupUser values(%d, %d, '%s')" , 
    groupid, userid, role.c_str());

    MySQL mysql;
    if (mysql.Connect()) {
        mysql.Update(sql);
    }
}

//查看群组信息
std::vector<Group> GroupModel::QueryGroups(int userid) {
    char sql[1024] = {0};
    sprintf(sql, "select a.id, a.groupname, a.groupdesc "
                "from AllGroup a "
                "inner join GroupUser b "
                "on a.id = b.groupid "
                "where b.userid = %d;" , userid);

    std::vector<Group> group_vec;

    MySQL mysql;
    if (mysql.Connect()) {
        std::unique_ptr<MYSQL_RES,decltype(&mysql_free_result)> res(mysql.Query(sql),mysql_free_result);
        if (res != nullptr) {
            MYSQL_ROW row;
            //查出所有群组信息
            while ((row = mysql_fetch_row(res.get())) != nullptr) {
                Group group;
                group.SetId(atoi(row[0]));
                group.SetName(row[1]);
                group.SetDesc(row[2]);
                group_vec.push_back(group);
            }
        }
    }

    ::memset(sql,0,sizeof(mysql));

    //对于每一个群组，再查询所有成员
    for(auto& group: group_vec) {
        sprintf(sql, "select u.id, u.name, u.state, g.grouprole "
                    "from User u "
                    "inner join GroupUser g "
                    "on u.id = g.userid "
                    "where g.groupid = %d;", group.GetId());

        std::unique_ptr<MYSQL_RES,decltype(&mysql_free_result)> res(mysql.Query(sql),mysql_free_result);
        if (res != nullptr) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res.get())) != nullptr) {
                GroupUser user;
                user.SetId(atoi(row[0]));
                user.SetName(row[1]);
                user.SetState(row[2]);
                user.SetRole(row[3]);
                group.GetUsers().push_back(user);
            }
        }
    }

    return group_vec;

}

//查询群用户，除了userid
std::vector<int> GroupModel::QueryGroupUsers(int userid, int groupid) {
    char sql[1024] = {0};

    sprintf(sql, "select userid "
                "from GroupUser "
                "where groupid = %d and userid != %d;", groupid, userid); 

    std::vector<int> id_vec;
    MySQL mysql;
    if (mysql.Connect()) {
        std::unique_ptr<MYSQL_RES,decltype(&mysql_free_result)> res(mysql.Query(sql),mysql_free_result);
        if (res != nullptr) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res.get())) != nullptr) {
                id_vec.push_back(atoi(row[0]));
            }
        }
    }
    return id_vec;
}

//查询指定id群信息
std::pair<std::vector<GroupUser>,std::vector<std::string>> GroupModel::QueryGroupById(int groupid) {
    char sql[1024] = {0};

    sprintf(sql, "select u.id, u.name, u.state, g.grouprole " 
                "from User u "
                "inner join GroupUser g "
                "on u.id = g.userid "
                "where g.groupid = %d;", groupid);

    //搜索所有成员
    std::vector<GroupUser> user_vec;
    MySQL mysql;
    if (mysql.Connect()) {
        std::unique_ptr<MYSQL_RES,decltype(&mysql_free_result)> res(mysql.Query(sql),mysql_free_result);
        if (res != nullptr) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res.get())) != nullptr) {
                GroupUser user;
                user.SetId(atoi(row[0]));
                user.SetName(row[1]);
                user.SetState(row[2]);
                user.SetRole(row[3]);
                user_vec.push_back(user);
            }
        }
    }


    //搜索群组信息
    ::memset(sql,0,sizeof(mysql));
    sprintf(sql, "select groupname, groupdesc "
                "from AllGroup "
                "where id = %d;",groupid);
    
    std::unique_ptr<MYSQL_RES,decltype(&mysql_free_result)> res(mysql.Query(sql),mysql_free_result);
    MYSQL_ROW row = mysql_fetch_row(res.get());
    
    std::vector<std::string> group_info;
    group_info.push_back(row[0]);
    group_info.push_back(row[1]);

    return {user_vec,group_info};
}