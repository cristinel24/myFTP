#pragma once

#include "common/utils.h"
#include "common/Database.h"
#include <fstream>

int server_socket;
Database users_db;

pthread_mutex_t thread_locker = PTHREAD_MUTEX_INITIALIZER;
std::vector<int> clients;

enum serverCommands {
    GET_LOGGED_USERS, 
    BAN, 
    UNBAN,
    GET_BANNED_USERS,
    LOGS, 
    None 
};

serverCommands mapServerCommands(const std::string& str) {
    if (str == "get_logged_users")  return GET_LOGGED_USERS;
    if (str == "get_banned_users")  return GET_BANNED_USERS;
    if (str == "ban")               return BAN;
    if (str == "unban")             return UNBAN;
    if (str == "logs")              return LOGS;
    return None;
}

void *handle_client(void *context);
void *client_manager(void *context);
void sigint_handler(int sig) {
    users_db.disconnectAllUsers();
    close(server_socket);
    exit(0);
}