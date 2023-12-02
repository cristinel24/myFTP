#include "include/client.h"

int main() {
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(DEFAULT_PORT);

    HANDLE((sock = socket(AF_INET, SOCK_STREAM, 0)));
    HANDLE(connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)));

    signal(SIGINT, sigint_handler);

    bool ok = login(sock);

    char temp[MAX_LOCATION_SIZE];
    msg_header hd;
    HANDLE(read(sock, &hd, sizeof(hd)));
    strcpy(temp, hd.data.path);

    std::string remoteLocation(temp);

    NULLCHECK(getcwd(cwd, sizeof(cwd)));

    FileManager fm(string(cwd), sock);

    std::string command;
    while (ok) {
        std::cout<< "remote: " << remoteLocation << '\n' << "local: " << fm.getCurrentPath() << '\n' << "> ";
        std::cin >> command;
        switch (mapClientCommands(command)) {
            case clientCommands::CDR: {
                std::string path;
                std::cin >> path;
                msg_header hd;
                hd.type = types::CD;
                strcpy(hd.data.path, path.c_str());
                HANDLE(write(sock, &hd, sizeof(hd)));
                HANDLE(read(sock, &hd, sizeof(hd)));

                CHECK_ERROR(hd.type);
                remoteLocation = string(hd.data.path);

                break;
            }
            case clientCommands::CDL: {
                std::string path;
                std::cin >> path;
                
                int ok = fm.cd(path);
                if (!ok) {
                    printf("Cannot open directory!\n");
                }
                break;
            }
            case clientCommands::LSR: {
                std::string args{};
                std::getline(std::cin, args);

                if (args.size() == 0){
                    args = ".";
                }
                msg_header hd;
                hd.type = types::LS;
                strcpy(hd.data.path, args.c_str());

                HANDLE(write(sock, &hd, sizeof(hd)));

                HANDLE(read(sock, &hd, sizeof(hd)));
                char *result = new char[hd.content_size]{};
                HANDLE(read(sock, result, hd.content_size));

                std::cout << result << '\n';
                delete[] result;
                break;
            }
            case clientCommands::LSL: {
                std::string args{};
                std::getline(std::cin, args);

                if (args.size() == 0){
                    args = ".";
                }
                std::cout << fm.ls(args) << '\n';
                break;
            }
            case clientCommands::STATR: {
                std::string path;
                std::cin >> path;
                msg_header hd;
                hd.type = types::STAT;
                strcpy(hd.data.path, path.c_str());
                HANDLE(write(sock, &hd, sizeof(hd)));

                HANDLE(read(sock, &hd, sizeof(hd)));
                char *result = new char[hd.content_size]{};
                HANDLE(read(sock, result, hd.content_size));

                std::cout << result << '\n';
                delete[] result;

                break;
            }
            case clientCommands::STATL: {
                std::string path;
                std::cin >> path;
                std::cout << fm.statFile(path) << '\n';

                break;
            }
            case clientCommands::UPLOADR: {
                std::string args{};
                std::getline(std::cin, args);
                auto tokens = split(args, ' ');
                std::string localPath;
                std::string remotePath = ".";
                if (tokens.size() == 2) 
                    localPath = tokens[1];
                else if (tokens.size() == 3) {
                    localPath = tokens[1];
                    remotePath = tokens[2];
                }
                fm.upload(localPath, remotePath);
                break;
            }
        }
        printf("\n");
    }

    return 0;
}

bool login(int sock) {
    login_header header;
    
    printf("Enter username: ");
    scanf("%s", username);

    char* password = (char *)calloc(MAX_PASSWORD_SIZE, 1);
    printf("Enter password: ");
    scanf("%s", password);

    strcpy(header.username, username);
    strcpy(header.password, hashFunction(password));

    delete password;

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

    printf("\n");
    return ok;
}