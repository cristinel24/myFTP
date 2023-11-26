#include "utils.h"

void send_payload(int fd, types type, const char* msg) {
    msg_header header;
    header.type = type;
    header.content_size = strlen(msg);
    
    HANDLE(write(fd, &header, sizeof(header)));
    HANDLE(write(fd, msg, header.content_size));
}

void send_connection_state(int fd, loginTypes type) {
    login_header header;
    header.type = type;
    HANDLE(write(fd, &header, sizeof(header)));
}