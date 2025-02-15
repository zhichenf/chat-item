#include "entity/user.h"

User::User(int id, std::string name, std::string password, std::string state) : 
id_(id),
name_(name),
password_(password),
state_(state) {}

void User::SetId(int id) {
    id_ = id;
}

void User::SetName(std::string name) {
    name_ = name;
}

void User::SetPwd(std::string password) {
    password_ = password;
}

void User::SetState(std::string state) {
    state_ = state;
}

int User::GetId() {
    return id_;
}

std::string User::GetName() {
    return name_;
}

std::string User::GetPwd() {
    return password_;
}

std::string User::GetState() {
    return state_;
}
