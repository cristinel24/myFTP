#include "include/server.h"

int main() {
    pthread_t tid;
    Logger logger("LOGS");

    signal(SIGINT, sigint_handler);

    void * args[2];
    args[0] = &logger;
    args[1] = &users_db;
    
    pthread_create(&tid, NULL, client_manager, args);

    std::string command;
    while (true) {
        printf("> ");
        std::cin >> command;
        switch (mapServerCommands(command)) {
            case serverCommands::GET_LOGGED_USERS: {
                int counter = 0;
                std::vector<std::string> users = users_db.getConnectedUsers();
                for (auto user : users) {
                    printf("%d. %s\n", ++counter, user.c_str());
                }
                printf("\n");
                break;
            }
            case serverCommands::BAN: {
                std::string user;
                std::cin >> user;
                HANDLE(users_db.ban(user));
                break;
            }
            case serverCommands::UNBAN: {
                std::string user;
                std::cin >> user;
                HANDLE(users_db.unban(user));
                break;
            }
            case serverCommands::GET_BANNED_USERS: {
                int counter = 0;
                std::vector<std::string> users = users_db.getBannedUsers();
                for (auto user : users) {
                    printf("%d. %s\n", ++counter, user.c_str());
                }
                printf("\n");
                break;
            }
            case serverCommands::GET_USERS: {
                int counter = 0;
                std::vector<std::string> users = users_db.getUsers();
                for (auto user : users) {
                    printf("%d. %s\n", ++counter, user.c_str());
                }
                printf("\n");
                break;
            }
            case serverCommands::LOGS: {
                std::ifstream fileName(logger.getFileName());
                std::string line;
                while (std::getline(fileName, line)) {
                    printf("%s\n", line.c_str());
                }
                break;
            }
            case serverCommands::ADD_USER: {
                std::string user;
                std::cin >> user;
                if (users_db.addUser(user) < 0) {
                    printf("Unable to add user %s. Maybe the user already exists?\n", user.c_str());
                }
                break;
            }
            case serverCommands::DELETE_USER: {
                std::string user;
                std::cin >> user;
                if (users_db.deleteUser(user) < 0) {
                    printf("Unable to delete user %s. Maybe the user doesn't exists?\n", user.c_str());
                }
                break;
            }
        }
        
    }

    close(server_socket);
    return 0;
}

void *client_manager(void *context) {

    auto logger         = reinterpret_cast<Logger*>(((void **)context)[0]);
    auto users_db       = reinterpret_cast<Database*>(((void **)context)[1]);

    int client_socket;
    sockaddr_in server_addr, client_addr;
    socklen_t client_addr_size;
    pthread_t tid;

    HANDLE(server_socket = socket(AF_INET, SOCK_STREAM, 0));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port        = htons(DEFAULT_PORT);

    int opt = 0;
    HANDLE(setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))); //too tired of "address already in use" :c
    HANDLE(bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)));
    HANDLE(listen(server_socket, MAX_CLIENTS));

    logger->Log(tid, logLevel::DEBUG, "Server is running at 127.0.0.1:%d\n", DEFAULT_PORT);

    while (true) {
        client_addr_size     = sizeof(client_addr);
        HANDLE(client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_addr_size));

        void * args[4];
        args[0] = logger;
        args[1] = users_db;
        args[2] = &client_socket;
        args[3] = &clients;

        pthread_create(&tid, NULL, handle_client, args);
    }
    return nullptr;
}

void *handle_client(void *context) {

    auto logger         = reinterpret_cast<Logger*>(((void **)context)[0]);
    auto users_db       = reinterpret_cast<Database*>(((void **)context)[1]);
    auto client_socket  = reinterpret_cast<int*>(((void **)context)[2]);
    auto clients        = reinterpret_cast<std::vector<int>*>(((void **)context)[3]);

    pthread_t tid = pthread_self();

    login_header resp;
    HANDLE(read(*client_socket, &resp, sizeof(resp)));
    if (!users_db->isUserAllowed(resp.username) || users_db->isUserConnected(resp.username)) {
        send_connection_state(*client_socket, loginTypes::FORBIDDEN);
        return nullptr;
    } else {
        std::string passwd;
        std::string username(resp.username);
        std::string user_pass(resp.password);
        
        HANDLE(users_db->select(username, passwd));

        //set passwd for new user
        if (passwd == "null") {
            users_db->updatePass(username, user_pass);
        }
        else if (passwd != user_pass) {
            send_connection_state(*client_socket, loginTypes::BAD_PASSWORD);
            return nullptr;
        }        
        users_db->connectUser(username);

        send_connection_state(*client_socket, loginTypes::CONNECTED);
    }

    clients->push_back(*client_socket);
    logger->Log(tid, logLevel::DEBUG, "New user connected: %s\n", resp.username);

    char cwd[MAX_LOCATION_SIZE];
    NULLCHECK(getcwd(cwd, sizeof(cwd)));

    msg_header hd;
    strcpy(hd.data.path, cwd);
    HANDLE(write(*client_socket, &hd, sizeof(hd)));

    FileManager fm(string(cwd), *client_socket);

    bool ok = true;
    while (ok) {
        msg_header header;
        HANDLE(read(*client_socket, &header, sizeof(header)));

        switch (header.type)
        {
            case types::CHECK_ACCESS: {
                msg_header hd;
                hd.type = users_db->isUserAllowed(string(header.username)) ? types::CHECK_ACCESS : types::ERROR;
                HANDLE(write(*client_socket, &hd, sizeof(hd)));

                if (hd.type == types::ERROR)
                    ok = !ok;
                break;
            }
            case types::USER_DISCONNECT:
                users_db->disconnectUser(header.username);
                ok = false;
                break;
            
            case types::CD: {
                int ok = fm.cd(string(header.data.path));
                logger->Log(tid, logLevel::INFO, "NEW LOCATION: %s\n", fm.getCurrentPath().c_str());
                msg_header hd;
                strcpy(hd.data.path, fm.getCurrentPath().c_str());
                ok? hd.type = types::SUCCESS :hd.type = types::ERROR;

                HANDLE(write(*client_socket, &hd, sizeof(hd)));
                break;
            }
            case types::LS: {
                std::string result{};
                result = fm.ls(string(header.data.path));
                send_payload(*client_socket, types::SUCCESS, result.c_str(), nullptr, nullptr);
                break;
            }
            case types::STAT: {
                std::string result{};
                result = fm.statFile(string(header.data.path));
                send_payload(*client_socket, types::SUCCESS, result.c_str(), nullptr, nullptr);
                break;
            }
            case types::MK_DIR: {
                bool result;
                result = fm.makeDirectory(string(header.data.path));
                types tp = result ? types::SUCCESS : types::ERROR;
                send_payload(*client_socket, tp, nullptr, nullptr, nullptr);
                break;
            }
            case types::UPLOAD: {
                bool result;
                result = fm.acceptUpload(header);
                types tp = result ? types::SUCCESS : types::ERROR;
                send_payload(*client_socket, tp, nullptr, nullptr, nullptr);
                break;
            }

            default:
                break;
        }
    }

    close(*client_socket);
    return nullptr;
}