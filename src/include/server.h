#pragma once

#include "common/utils.h"
#include "common/Database.h"
#include "common/Logger.h"
#include "common/FileManager.h"
#include <fstream>

int server_socket;
Database users_db;

pthread_mutex_t thread_locker = PTHREAD_MUTEX_INITIALIZER;
std::vector<int> clients;

enum serverCommands {
    GET_LOGGED_USERS,
    ADD_USER,
    DELETE_USER,
    GET_USERS,
    BAN, 
    UNBAN,
    GET_BANNED_USERS,
    LOGS, 
    None 
};

serverCommands mapServerCommands(const std::string& str) {
    if (str == "logged")        return GET_LOGGED_USERS;
    if (str == "banned")        return GET_BANNED_USERS;
    if (str == "users")         return GET_USERS;
    if (str == "ban")           return BAN;
    if (str == "unban")         return UNBAN;
    if (str == "logs")          return LOGS;
    if (str == "add")           return ADD_USER;
    if (str == "delete")        return DELETE_USER;
    
    return None;
}

void *handle_client(void *context);
void *client_manager(void *context);

void sigint_handler(int sig) {
    HANDLE(users_db.disconnectAllUsers());
    HANDLE(close(server_socket));
    exit(0);
}