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
    string fileName;
    mutex sem; 

    public:
    Logger(const string& fileName) : fileName(fileName) {}

    string getFileName();
    void Log(pthread_t tid, logLevel level, const char* format, ...);
};