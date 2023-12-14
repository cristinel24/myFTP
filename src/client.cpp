#include "include/client.h"

int main() {
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(DEFAULT_PORT);

    HANDLE((sock = socket(AF_INET, SOCK_STREAM, 0)));
    HANDLE(connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)));

    signal(SIGINT, sigint_handler);

    bool ok = login(sock);
    if (!ok) exit(0);

    char temp[MAX_LOCATION_SIZE];
    msg_header hd;
    HANDLE(read(sock, &hd, sizeof(hd)));
    strcpy(temp, hd.path);

    std::string remoteLocation(temp);

    NULLCHECK(getcwd(cwd, sizeof(cwd)));

    FileManager fm(string(cwd), sock);

    std::string command;
    while (true) {
        msg_header hd;
        hd.type = types::CHECK_ACCESS;
        strcpy(hd.username, username);

        HANDLE(write(sock, &hd, sizeof(hd)));
        HANDLE(read(sock, &hd, sizeof(hd)));
        if (hd.type == types::ERROR) {
            printf("You dont have access to this server!\n");
            exit(0);
        }

        std::cout<< "remote: " << remoteLocation << '\n' << "local: " << fm.getCurrentPath() << '\n' << "> ";
        std::cin >> command;
        switch (mapClientCommands(command)) {
            case clientCommands::CDR: {
                std::string path;
                std::cin >> path;
                msg_header hd;
                hd.type = types::CD;
                strcpy(hd.path, path.c_str());

                HANDLE(write(sock, &hd, sizeof(hd)));
                HANDLE(read(sock, &hd, sizeof(hd)));

                if (hd.type == types::ERROR) {
                    printf("Cannot open directory!\n");
                }
                remoteLocation = string(hd.path);

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
                hd.content_size = 0;
                strcpy(hd.path, args.c_str());

                HANDLE(write(sock, &hd, sizeof(hd)));


                HANDLE(read(sock, &hd, sizeof(hd)));

                char *result = (char *)calloc(hd.content_size, 1);
                HANDLE(read(sock, result, hd.content_size));

                std::cout << result << '\n';

                free(result);
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
                strcpy(hd.path, path.c_str());
                HANDLE(write(sock, &hd, sizeof(hd)));

                HANDLE(read(sock, &hd, sizeof(hd)));
                char *result = (char *)calloc(hd.content_size, 1);
                HANDLE(read(sock, result, hd.content_size));

                std::cout << result << '\n';
                free(result);

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
                std::string remotePath;
                if (tokens.size() == 2) {
                    localPath = tokens[1];
                    remotePath = fm.getFileName(tokens[1]);
                }
                else if (tokens.size() == 3) {
                    localPath = tokens[1];
                    remotePath = tokens[2];
                }
                fm.transfer(localPath, remotePath, types::UPLOAD);
                break;
            }
            case clientCommands::DOWNLOADR: {
                std::string args{};
                std::getline(std::cin, args);
                auto tokens = split(args, ' ');
                std::string localPath;
                std::string remotePath;
                if (tokens.size() == 2) {
                    remotePath = tokens[1];
                    localPath = fm.getFileName(tokens[1]);
                }
                else if (tokens.size() == 3) {
                    remotePath = tokens[1];
                    localPath = tokens[2];
                }

                msg_header hd;
                hd.type = types::DOWNLOAD;
                strcpy(hd.path, remotePath.c_str());
                hd.content_size = localPath.size();

                HANDLE(write(sock, &hd, sizeof(hd)));
                HANDLE(write(sock, localPath.c_str(), hd.content_size));
                msg_header header;
                header.type = types::SUCCESS;

                while(header.type != types::STOP) {
                    HANDLE(read(sock, &header, sizeof(header)));
                    switch(header.type) {
                        case types::MK_DIR: {
                           bool result;
                            result = fm.makeDirectory(string(header.path));
                            types tp = result ? types::SUCCESS : types::ERROR;
                            send_payload(sock, tp, nullptr, nullptr, nullptr);
                            break;
                        }
                        case types::DOWNLOAD: {
                            bool result = false;
                            result = fm.acceptTransfer(header);
                            if (!result) {
                                printf("There was an error downloading this file!\n");
                            }
                            break;
                        }
                        default:
                            break;
                    }
                }
                
                break;
            }
            case clientCommands::QUIT: {
                send_payload(sock, types::USER_DISCONNECT, nullptr, username, nullptr);
                close(sock);
                exit(0);
            }
            case clientCommands::HELP: {
                std::cout << help << '\n';
                break;
            }
            default:
                std::cout << help << '\n';
                break;
        }
        printf("\n");
    }

    printf("You were banned from this server.\n");

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

    free(password);

    send(sock, &header, sizeof(header), 0);
    HANDLE(read(sock, &header, sizeof(header)));

    bool ok = (header.type == loginTypes::CONNECTED)? true : false;

    switch (header.type)
    {
        case loginTypes::FORBIDDEN:
            printf("You don't have acces to this server :c\n");
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