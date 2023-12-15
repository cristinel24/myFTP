#pragma once

#include "Logger.h"
#include "utils.h"

#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>
#include <iomanip>
#include <memory>
#include <filesystem>

#include <vector>
#include <string>
#include <algorithm>
#include <cctype>

#define INVALID_PATH "ERROR: INVALID PATH!"
#define MAX_SIZE 1024
#define LS_WIDTH 40

using namespace std;

class FileManager {
    string currentPath;
    int remote;
    Logger* logger;
    pthread_t tid;
    pthread_mutex_t mutex;

    string getPermissionsAsString(uint32_t perms);

public:
    FileManager(const string& path, int remote) : currentPath(path), remote(remote) {};
    
    string getCurrentPath();
    string getFileName(const string& _path);

    string ls(const string& path);
    bool cd(const string& path);

    string statFile(const string& path);
    bool makeDirectory(const string& path);

    bool transfer(const string& local, const string& remote, const types type);
    bool acceptTransfer(const msg_header header);

};
