#pragma once

#include <fstream>
#include <mutex>
#include <cstdarg>

#define MAX_LOG_LINE 4096

using namespace std;

enum logLevel {
    DEBUG,
    INFO,
    ERR
};

class Logger {
    string fileLocation;
    mutex sem; 

    public:
    Logger(const string& fileLocation) : fileLocation(fileLocation) {}

    string getFileLocation();
    void Log(pthread_t tid, logLevel level, const char* format, ...);
};