#pragma once

#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <arpa/inet.h>

#include <iostream>
#include <stdlib.h>
#include <cstdint>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <ctype.h>
#include <signal.h>
#include <utmp.h>
#include <time.h>
#include <pthread.h>

//STD
#include <vector>
#include <string>
#include <stdexcept>

#define DEFAULT_PORT 11111
#define MAX_CLIENTS 16

#define HASH_CONSTANT 666

#define DEFAULT_MAX 4096
#define MAX_PASSWORD_SIZE DEFAULT_MAX
#define MAX_USERNAME_SIZE DEFAULT_MAX
#define MAX_FILENAME_SIZE DEFAULT_MAX
#define MAX_LOCATION_SIZE DEFAULT_MAX
#define CHUNK MAX_SIZE

#define ANSI_COLOR_RED          "\x1b[31m"
#define ANSI_COLOR_GREEN        "\x1b[32m"
#define ANSI_COLOR_YELLOW       "\x1b[32m"
#define ANSI_COLOR_BLUE         "\x1b[34m"
#define ANSI_COLOR_LIGHT_GRAY   "\033[90m"
#define ANSI_STYLE_ITALIC       "\x1b[3m"
#define ANSI_RESET              "\x1b[0m"

#define HANDLE(f) if ((f) < 0) {printf("FILE: %s\n", __FILE__); perror(#f); exit(1);}
#define NULLCHECK(f) if ((f) == NULL) {printf("FILE: %s\n", __FILE__); perror(#f); exit(1);}

#define HANDLE_NO_EXIT(f) if ((f) < 0) {printf("FILE: %s\n", __FILE__); perror(#f);}
#define NULLCHECK_NO_EXIT(f) if ((f) == NULL) {printf("FILE: %s\n", __FILE__); perror(#f);}

enum types { 
    NONE,
    ERROR, 
    UPLOAD,
    DOWNLOAD,
 
    CD, 
    LS, 
    STAT,
    MK_DIR,

    USER_DISCONNECT,
    STOP,
    CHECK_ACCESS,
    SUCCESS
};

enum loginTypes {
    LOGIN, 
    FORBIDDEN, 
    BAD_PASSWORD,
    CONNECTED
};

struct msg_header {
    enum types type = types::NONE;
    size_t content_size = 0;
    char username[MAX_USERNAME_SIZE]{0};
    char path[MAX_LOCATION_SIZE]{0};
};

struct login_header {
    enum loginTypes type;
    char username[MAX_USERNAME_SIZE]{0};
    char password[MAX_PASSWORD_SIZE]{0};
};

std::vector<std::string> split(const std::string &content, char del);

void send_payload(int fd, types type, const char* msg, const char* username, const char* path);
void send_connection_state(int fd, loginTypes type);

