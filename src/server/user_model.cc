#include "user_model.h"

#include <iostream>

#include "db/db.h"

bool UserModel::Insert(User& user) {
    char sql[1024] = {0};
    sprintf(sql,"insert into User(name, password, state) values ('%s', '%s', '%s')", 
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