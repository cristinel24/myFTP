#include "utils.h"

void send_payload(int fd, types type, const char* msg, const char* username, FileData* data) {
    msg_header header;
    header.type = type;
    msg != nullptr ? header.content_size = strlen(msg) :  header.content_size = 0;
    if (username != nullptr)
        strcpy(header.username, username);

    if (data != nullptr) {
        strcpy(header.data.fileName, data->fileName);
        strcpy(header.data.path, data->path);
    }

    HANDLE(write(fd, &header, sizeof(header)));
    if (header.content_size)
        HANDLE(write(fd, msg, header.content_size));
}

void send_connection_state(int fd, loginTypes type) {
    login_header header;
    header.type = type;
    HANDLE(write(fd, &header, sizeof(header)));
}

std::vector<std::string> split(const std::string &content, char del) {
    std::vector<std::string> tokens;
    uint start = 0;
    uint end = content.find(del);

    while (end < content.size() + 1) {
        tokens.push_back(content.substr(start, end - start));
        start = end + 1;
        end = content.find(del, start);
    }

    tokens.push_back(content.substr(start, end));
    return tokens;
}
