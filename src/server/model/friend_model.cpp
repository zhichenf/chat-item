#include "model/friend_model.h"

#include <string.h>

#include <memory>

#include "db/db.h"

//添加好友关系
void FriendModel::Insert(int userid, int friendid) {
    char sql[1024] = {0};
    sprintf(sql, "insert into Friend values (%d, %d);", userid, friendid);

    MySQL mysql;
    if (mysql.Connect()) {
        mysql.Update(sql);
    }

    ::memset(sql,0,sizeof(sql));

    sprintf(sql, "insert into Friend values (%d, %d);", friendid, userid);
    mysql.Update(sql);
}

//返回用户好友列表
std::vector<User> FriendModel::Query(int userid) {
    char sql[1024] = {0};
    //select u.id, u.name, u.state
    //from User u
    //inner join Friend f
    //on u.id = f.friendid
    //where f.userid = %d
    sprintf(sql,"select u.id, u.name, u.state "
                "from User u "
                "inner join Friend f "
                "on u.id = f.friendid "
                "where f.userid = %d;", userid);

    MySQL mysql;
    std::vector<User> vec;
    if (mysql.Connect()) {
        std::unique_ptr<MYSQL_RES,decltype(&mysql_free_result)> res(mysql.Query(sql),mysql_free_result);
        if (res != nullptr) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res.get())) != nullptr) {
                User user;
                user.SetId(atoi(row[0]));
                user.SetName(row[1]);
                user.SetState(row[2]);
                vec.push_back(user);
            }
        }
    } 
    return vec;
    
}