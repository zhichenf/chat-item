#include "offline_message_model.h"

#include <memory>

#include "db/db.h"

//存储用户的离线消息
void OfflineMsgModel::Insert(int usrid, std::string msg) {
    char sql[1024] = {0};
    sprintf(sql,"insert into OfflineMessage values (%d, '%s');", 
    usrid,msg.c_str());

    MySQL mysql;
    if (mysql.Connect()) {
        mysql.Update(sql);
    } 
}

//删除用户的离线消息
void OfflineMsgModel::Remove(int usrid) {
    char sql[1024] = {0};
    sprintf(sql,"delete from OfflineMessage where userid = %d;", usrid);

    MySQL mysql;
    if (mysql.Connect()) {
        mysql.Update(sql);
    } 
}

//查询用户的离线消息
std::vector<std::string> OfflineMsgModel::Query(int usrid) {
    char sql[1024] = {0};
    sprintf(sql,"select message from OfflineMessage where userid = %d;", usrid);

    MySQL mysql;
    std::vector<std::string> vec;
    if (mysql.Connect()) {
        std::unique_ptr<MYSQL_RES,decltype(&mysql_free_result)> res(mysql.Query(sql),mysql_free_result);
        if (res != nullptr) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res.get())) != nullptr) {
                vec.push_back(row[0]);
            }
        }
    } 
    return vec;
}