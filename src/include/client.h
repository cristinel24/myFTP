#pragma once

#include "common/utils.h"
#include "common/FileManager.h"


int sock = 0;
sockaddr_in server_addr;
char username[MAX_USERNAME_SIZE];
char cwd[MAX_LOCATION_SIZE];

#define CHECK_ERROR(var) \
    if ((var) == types::ERROR) { \
        std::cout << "ERROR: "; \
        break; \
    }

enum clientCommands {
    CDL, 
    CDR,
    LSL,
    LSR,
    STATL,
    STATR,
    UPLOADR,
    DOWNLOADR,
    QUIT,
    None 
};

clientCommands mapClientCommands(const std::string& str) {
    if (str == "cd")        return clientCommands::CDL;
    if (str == "cdr")       return clientCommands::CDR;
    if (str == "ls")        return clientCommands::LSL;
    if (str == "lsr")       return clientCommands::LSR;
    if (str == "stat")      return clientCommands::STATL;
    if (str == "statr")     return clientCommands::STATR;
    if (str == "up")        return clientCommands::UPLOADR;
    if (str == "down")      return clientCommands::DOWNLOADR;
    if (str == "quit")      return clientCommands::QUIT;
    return clientCommands::None;
}

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