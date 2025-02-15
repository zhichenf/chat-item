#include "model/user_model.h"

#include <iostream>
#include <memory>

#include "db/db.h"

bool UserModel::Insert(User& user) {
    char sql[1024] = {0};
    sprintf(sql,"insert into User(name, password, state) values ('%s', '%s', '%s');", 
    user.GetName().c_str(), user.GetPwd().c_str(), user.GetState().c_str());

    MySQL mysql;
    if (mysql.Connect()) {
        if(mysql.Update(sql)) {
            //获取插入成功的用户的id
            user.SetId(mysql_insert_id(mysql.GetConnection()));
            return true;
        }
    }

    return false;   
}

User UserModel::QueryId(int id) {
    char sql[1024] = {0};
    sprintf(sql,"select * from User where id = %d;", id);

    MySQL mysql;
    if (mysql.Connect()) {
        //MYSQL_RES* res = mysql.Query(sql);
        std::unique_ptr<MYSQL_RES,decltype(&mysql_free_result)> res(mysql.Query(sql),mysql_free_result);
        if (res != nullptr) {
            MYSQL_ROW row = mysql_fetch_row(res.get());
            if (row != nullptr) {
                User user;
                user.SetId(atoi(row[0]));
                user.SetName(row[1]);
                user.SetPwd(row[2]);
                user.SetState(row[3]);
                //mysql_free_result(res);
                return user;
            }
            //mysql_free_result(res);
        }
    }
    return User();
}

User UserModel::QueryName(std::string name){
    char sql[1024] = {0};
    sprintf(sql,"select * from User where name = '%s';", name.c_str());

    MySQL mysql;
    if (mysql.Connect()) {
        std::unique_ptr<MYSQL_RES,decltype(&mysql_free_result)> res(mysql.Query(sql),mysql_free_result);
        if (res != nullptr) {
            MYSQL_ROW row = mysql_fetch_row(res.get());
            if (row != nullptr) {
                User user;
                user.SetId(atoi(row[0]));
                user.SetName(row[1]);
                user.SetPwd(row[2]);
                user.SetState(row[3]);
                return user;
            }
        }
    }
    return User();
}

bool UserModel::UpdateState(User& user) {
    char sql[1024] = {0};
    sprintf(sql,"update User set state = '%s' where id = %d;", user.GetState().c_str(), user.GetId());
    MySQL mysql;
    if (mysql.Connect()) {
        if (mysql.Update(sql)) {
            return true;
        }
    }
    return false;
}

//重置用户的状态信息
void  UserModel::ResetState() {
    char sql[1024] = "update User set state = 'offline' where state = 'online';";
    MySQL mysql;
    if (mysql.Connect()) {
        mysql.Update(sql);
    }
}