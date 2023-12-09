#pragma once

#include "common/utils.h"
#include "common/FileManager.h"


int sock = 0;
sockaddr_in server_addr;
char username[MAX_USERNAME_SIZE];
char cwd[MAX_LOCATION_SIZE];

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
    HELP,
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
    if (str == "help")      return clientCommands::HELP;
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

const std::string help = "AVAILABLE COMMANDS: \n" \
                         "cd        -> Change local path (usage: cd [path]) \n" \
                         "cdr       -> Change remote path (usage: cdr [path]) \n" \
                         "ls        -> List current local directory (usage: ls [optional: path]) \n" \
                         "lsr       -> List current remote directory (usage: lsr [optional: path]) \n" \
                         "stat      -> Get local file metadata (usage: stat [path]) \n" \
                         "statr     -> Get remote file metadata (usage: stat [path]) \n" \
                         "up        -> Upload a file to the remote server (usage: up [localPath] [remotePath]) \n" \
                         "down      -> Download a file from the remote server (usage: down [remotePath] [localPath]) \n" \
                         "help      -> Print this message \n";
