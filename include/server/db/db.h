#ifndef DB_H_
#define DB_H_

#include <string>

#include <mysql/mysql.h>


 // 数据库操作类
class MySQL
{
public:
    // 初始化数据库连接
    MySQL();
    // 释放数据库连接资源
    ~MySQL();
    // 连接数据库
    bool Connect();
    // 更新操作
    bool Update(std::string sql);
    // 查询操作
    MYSQL_RES* Query(std::string sql);
    //获取连接
    MYSQL* GetConnection();

private:
    MYSQL* conn_;
};

#endif

