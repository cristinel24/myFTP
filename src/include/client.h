#pragma once

#include "common/utils.h"


int sock = 0;
sockaddr_in server_addr;
char username[MAX_USERNAME_SIZE];
char password[MAX_PASSWORD_SIZE];

const char * hashFunction(const std::string &password) {
    unsigned long long hash = HASH_CONSTANT;
    for (char c : password) {
        hash = ((hash << 3) + hash) + c;
    }
    return std::to_string(hash).c_str();
}


bool login(int socket);