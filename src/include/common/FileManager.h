#pragma once

#include "Logger.h"
#include "utils.h"

#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>
#include <memory>
#include <filesystem>

#include <vector>
#include <string>
#include <algorithm>
#include <cctype>

#define INVALID_PATH "ERROR: INVALID PATH!"
#define MAX_SIZE 1024

using namespace std;

class FileManager {
    string currentPath;
    int remote;
    Logger* logger;
    pthread_t tid;

    string getPermissionsAsString(uint32_t perms);

public:
    FileManager(const string& path, int remote) : currentPath(path), remote(remote) {};
    
    string getCurrentPath();

    string ls(const string& path);
    bool cd(const string& path);

    string statFile(const string& path);
    bool makeDirectory(const string& path);

    bool upload(const string& local, const string& remote);
    bool acceptUpload(const msg_header header);

    bool download(const string& remote, const string& local);
};
