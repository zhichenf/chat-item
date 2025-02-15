#include "entity/group_user.h"

void GroupUser::SetRole(std::string role) {
    role_ = role;
}

std::string GroupUser::GetRole() {
    return role_;
}