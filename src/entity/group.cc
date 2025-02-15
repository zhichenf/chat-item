#include "entity/group.h"

Group::Group(int id, std::string name, std::string desc) 
: id_(id), name_(name), desc_(desc) {}
    
void Group::SetId(int id) {
    id_ = id;
}
void Group::SetName(std::string name) {
    name_ = name;
}

void Group::SetDesc(std::string desc) {
    desc_ = desc;
}

int Group::GetId() {
    return id_;
}

std::string Group::GetName() {
    return name_;
}

std::string Group::GetDesc() {
    return desc_;
}

std::vector<GroupUser>& Group::GetUsers() {
    return users_;
}
