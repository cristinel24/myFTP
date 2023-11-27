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