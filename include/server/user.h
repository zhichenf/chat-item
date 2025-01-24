#ifndef USER_H_
#define USER_H_

#include <string>

class User{
public:
    User(int id = -1, std::string name = "", std::string password = "", std::string state = "offline");
    ~User() = default;
    void SetId(int id);
    void SetName(std::string name);
    void SetPwd(std::string password);
    void SetState(std::string state);
    int GetId();
    std::string GetName();
    std::string GetPwd();
    std::string GetState();

private:
    int id_;
    std::string name_;
    std::string password_;
    std::string state_;
};

#endif