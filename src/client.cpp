#include "include/client.h"

int main() {
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(DEFAULT_PORT);

    HANDLE((sock = socket(AF_INET, SOCK_STREAM, 0)));
    HANDLE(connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)));

    bool ok = login(sock);

    while (ok) {
        printf("YUPPY\n");
    }

    close(sock);
    return 0;
}

bool login(int sock) {
    login_header header;

    printf("Enter username: ");
    scanf("%s", username);
    printf("Enter password: ");
    scanf("%s", password);

    strcpy(header.username, username);
    strcpy(header.password, hashFunction(password));

    send(sock, &header, sizeof(header), 0);
    HANDLE(read(sock, &header, sizeof(header)));

    bool ok = (header.type == loginTypes::CONNECTED)? true : false;

    switch (header.type)
    {
        case loginTypes::FORBIDDEN:
            printf("You were banned from this server :c\n");
            break;

        case loginTypes::BAD_PASSWORD:
            printf("Wrong password!\n");
            break;
        default:
            break;
    }

    return ok;
}