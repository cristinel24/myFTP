#include "../include/common/Logger.h"
 
string Logger::getFileLocation(){
    return fileLocation;
} 

void Logger::Log(pthread_t tid, logLevel level, const char* format, ...) {
    lock_guard<mutex> guard(sem);
    
    ofstream logFile(fileLocation, ios::app);

    char buffer[MAX_LOG_LINE];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    switch (level) {
        case logLevel::DEBUG:
            logFile << "[DEBUG] " << "[TID: " << tid << "]: " << buffer; 
            break;
        case logLevel::INFO:
            logFile << "[INFO] " << "[TID: " << tid << "]: "  << buffer;
            break;
        case logLevel::ERR:
            logFile << "[ERROR] " << "[TID: " << tid << "]: "  << buffer;
            break;
    }
    logFile.close();
}
 