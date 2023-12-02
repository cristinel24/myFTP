#include "../include/common/Database.h"

const char *base = "CREATE TABLE IF NOT EXISTS users ("
                   "username TEXT PRIMARY KEY, "
                   "password TEXT, "
                   "allowed BOOLEAN DEFAULT TRUE, "
                   "connected BOOLEAN DEFAULT TRUE);";

Database::Database() {
    connection = sqlite3_open("users.db", &db);
    if (connection != SQLITE_OK) {
        sqlite3_close(db);
        char msg[128];
        sprintf(msg, "Unable to open database: %s", sqlite3_errmsg(db));
        throw runtime_error(msg);
    } else {
        if (sqlite3_exec(db, base, nullptr, nullptr, &errMsg) != SQLITE_OK) {
            char msg[128];
            sprintf(msg, "Unable to open database: %s", errMsg);
            sqlite3_close(db);
            sqlite3_free(errMsg);
            throw runtime_error(msg);
        }
    }
}

Database::~Database() {
    if (db) {
        sqlite3_close(db);
    }
}

int Database::addUser(const string &username) {
    string query = "INSERT INTO users (username, allowed, connected) "
                   "VALUES ('" + username + "', 1, 0);";

    if (sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        sqlite3_free(errMsg);
        return -1; 
    }
    return 1; 
}

int Database::deleteUser(const string &username) {
    string query = "DELETE FROM users "
                   "WHERE username = '" + username + "';";

    if (sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        sqlite3_free(errMsg);
        return -1;
    }
    return 1;
}

int Database::select(const string &username, string &value) {
    string query = "SELECT password "
                   "FROM users "
                   "WHERE username = '" + username + "';";

    char **results = nullptr;
    int rows, columns;
    
    if (sqlite3_get_table(db, query.c_str(), &results, &rows, &columns, &errMsg) == SQLITE_OK) {
        if (rows) {
            if (results[1] == NULL)
                value = "null"; 
            else value = results[1]; 
        } else {
            value = ""; 
        }
        sqlite3_free_table(results);
        return 0; 
    } else {
        sqlite3_free(errMsg);
        return -1; 
    }
}

int Database::updatePass(const string& username, const string& value) {
    sqlite3_stmt *stmt;
    string query = "UPDATE users "
                        "SET password = '" + value + "' "                
                        "WHERE username = ?";

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) 
        return -1;

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

    int status = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (status == SQLITE_DONE)
        return 0;
    return -1;
}

bool Database::isUserAllowed(const string &username) {
    string query = "SELECT allowed "
                   "FROM users "
                   "WHERE username = '" + username + "';";

    char **results = nullptr;
    int rows = 0, columns = 0;

    if (sqlite3_get_table(db, query.c_str(), &results, &rows, &columns, &errMsg) == SQLITE_OK) {
        if (rows) {
            int allowed = stoi(results[1]); 
            sqlite3_free_table(results);
            return allowed != 0; 
        } else {
            return false; 
        }
    } else {
        sqlite3_free(errMsg);
        return false; 
    }
}

bool Database::isUserConnected(const string & username) {
    string query = "SELECT connected "
                   "FROM users "
                   "WHERE username = '" + username + "';";

    char **results = nullptr;
    int rows = 0, columns = 0;

    if (sqlite3_get_table(db, query.c_str(), &results, &rows, &columns, &errMsg) == SQLITE_OK) {
        if (rows) {
            int connected = stoi(results[1]); 
            sqlite3_free_table(results);
            return connected != 0; 
        } else {
            return false;
        }
    } else {
        sqlite3_free(errMsg);
        return false; 
    }
}

int Database::connectUser(const string &username) {
    sqlite3_stmt *stmt;
    const char *query = "UPDATE users "
                        "SET connected = 1 "                
                        "WHERE username = ?";

    if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) 
        return -1;

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

    int status = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (status == SQLITE_DONE)
        return 0;
    return -1;
}

vector<string> Database::getConnectedUsers() {
    vector<string> connectedUsers;
    sqlite3_stmt *stmt;
    const char *query = "SELECT username "
                        "FROM users "
                        "WHERE connected = 1";

    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        return connectedUsers;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        string username(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
        connectedUsers.push_back(username);
    }

    sqlite3_finalize(stmt);
    return connectedUsers;
}

vector<string> Database::getUsers() {
    vector<string> users;
    sqlite3_stmt *stmt;
    const char *query = "SELECT username "
                        "FROM users ";

    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        return users;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        string username(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
        users.push_back(username);
    }

    sqlite3_finalize(stmt);
    return users;
}

int Database::disconnectAllUsers() {
    const char *query = "UPDATE users "
                        "SET connected = 0";

    if (sqlite3_exec(db, query, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        sqlite3_free(errMsg);
        return -1; 
    }
    return 0; 
}

int Database::disconnectUser(const std::string &username) {
    sqlite3_stmt *stmt;
    const char *query = "UPDATE users "
                        "SET connected = 0 "                
                        "WHERE username = ?";

    if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) 
        return -1;

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

    int status = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (status == SQLITE_DONE)
        return 0;
    return -1;
}

int Database::ban(const string &username) {
    string query = "UPDATE users "
                   "SET allowed = 0, connected = 0 "
                   "WHERE username = '" + username + "';";

    if (sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        
        query = "INSERT INTO users (username, connection, allowed) "
                "VALUES ('" + username + "', 0, 0) "
                "ON CONFLICT(username) DO NOTHING;";
                
        if (sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
            sqlite3_free(errMsg);
            return -1; 
        }
    }
    return 0;
}

int Database::unban(const string &username) {
    string query = "UPDATE users "
                   "SET allowed = 1 "
                   "WHERE username = '" + username + "';";

    if (sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        return -1;
    }
    return 0;
}

vector<string> Database::getBannedUsers() {
    vector<string> bannedUsers;
    sqlite3_stmt *stmt;
    const char *query = "SELECT username "
                        "FROM users "
                        "WHERE allowed = 0;";

    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        return bannedUsers;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        string username(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
        bannedUsers.push_back(username);
    }

    sqlite3_finalize(stmt);
    return bannedUsers;
}
