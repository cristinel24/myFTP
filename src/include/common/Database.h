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
    
    int addUser(const string &username);
    int deleteUser(const string &username);

    int select(const string &username, string &value);
    int updatePass(const string& username, const string& value);
    bool isUserAllowed(const string &username);
    bool isUserConnected(const string & username);

    int connectUser(const string &username);
    vector<string> getConnectedUsers();
    vector<string> getUsers();
    int disconnectAllUsers();
    int disconnectUser(const std::string &username);

    int ban(const string &username);
    int unban(const string &username);
    vector<string> getBannedUsers();
};