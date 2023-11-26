#pragma once

#include <iostream>
#include <sqlite3.h>
#include <vector>

using namespace std;

class Database {

    sqlite3 *db;
    int connection;
    char *errMsg;

    public:

    Database();
    ~Database();
    
    int insert(const string &username, const string &password);
    int select(const string &username, string &value);
    bool isUserAllowed(const string &username, const string &password);

    int connectUser(const string &username);
    vector<string> getConnectedUsers();
    int disconnectAllUsers();
    int disconnectUser(const std::string &username);

    int ban(const string &username);
    int unban(const string &username);
    vector<string> getBannedUsers();
};