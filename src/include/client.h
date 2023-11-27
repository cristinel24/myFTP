#pragma once

#include "common/utils.h"


int sock = 0;
sockaddr_in server_addr;
char username[MAX_USERNAME_SIZE];

const char * hashFunction(const std::string &password) {
    unsigned long long hash = HASH_CONSTANT;
    for (char c : password) {
        hash = ((hash << 3) + hash) + c;
    }
    return std::to_string(hash).c_str();
}

void sigint_handler(int sig) {
    send_payload(sock, types::USER_DISCONNECT, nullptr, username, nullptr);
    close(sock);
    exit(0);
}

bool login(int socket);