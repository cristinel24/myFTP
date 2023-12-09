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
    REMOVE_USER,
    GET_USERS,
    BAN, 
    UNBAN,
    GET_BANNED_USERS,
    LOGS, 
    HELP,
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
    if (str == "remove")        return REMOVE_USER;
    if (str == "help")          return HELP;
    
    return None;
}

void *handle_client(void *context);
void *client_manager(void *context);

void sigint_handler(int sig) {
    HANDLE(users_db.disconnectAllUsers());
    HANDLE(close(server_socket));
    exit(0);
}

const std::string help = "AVAILABLE COMMANDS: \n" \
                         "users     -> Get a list of all the users in the database \n" \
                         "logged    -> Get a list of all connected users \n" \
                         "add       -> Add a user to the database (usage: add [username]) \n" \
                         "remove    -> Remove a user from the database (usage: remove [username]) \n" \
                         "ban       -> Ban a user (usage: ban [username]) \n" \
                         "unban     -> Unban a user (usage: unban [username]) \n" \
                         "banned    -> Get a list of all banned users \n" \
                         "logs      -> Print the LOGS content to the terminal \n" \
                         "help      -> Print this message \n";
