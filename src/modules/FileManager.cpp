#include "../include/common/FileManager.h"

string FileManager::getPermissionsAsString(uint32_t perms) {
    string permissions{};
    permissions.push_back((perms & S_IRUSR) ? 'r' : '-');
    permissions.push_back((perms & S_IWUSR) ? 'w' : '-');
    permissions.push_back((perms & S_IXUSR) ? 'x' : '-');
    permissions.push_back((perms & S_IRGRP) ? 'r' : '-');
    permissions.push_back((perms & S_IWGRP) ? 'w' : '-');
    permissions.push_back((perms & S_IXGRP) ? 'x' : '-');
    permissions.push_back((perms & S_IROTH) ? 'r' : '-');
    permissions.push_back((perms & S_IWOTH) ? 'w' : '-');
    permissions.push_back((perms & S_IXOTH) ? 'x' : '-');
    return permissions;
}

bool FileManager::compareFiles(const dirent* file_a, const dirent* file_b) {
    if (file_a->d_type != file_b->d_type) {
        return file_a->d_type == DT_DIR;
    }
    return strcmp(file_a->d_name, file_b->d_name) < 0;
}

string FileManager::getCurrentPath() {
    return currentPath;
}

string FileManager::getFileName(const string& _path) {
    return filesystem::path(_path).filename();
}

string FileManager::ls(const string& path) {
    struct dirent *item;
    ostringstream result;
    string type;
    vector<dirent*> items;

    string cleanPath = path;

    cleanPath.erase(remove_if(cleanPath.begin(), cleanPath.end(), [](char c) {
        return isspace(c);
    }), cleanPath.end());

    DIR *dir = opendir(cleanPath.c_str());
    if (dir != nullptr) {

        while ((item = readdir(dir)) != nullptr) {
            items.push_back(item);
        }
        sort(items.begin(), items.end(), FileManager::compareFiles);

        for (const auto& item: items) {
            if (strcmp(item->d_name, ".") != 0 && strcmp(item->d_name, "..") != 0) {
                
                switch (item->d_type)
                {
                    case DT_DIR:        
                        result << ANSI_COLOR_BLUE << std::left << std::setw(LS_WIDTH) << item->d_name;
                        type = "- directory";  break;

                    case DT_REG:        
                        result << ANSI_COLOR_GREEN << std::left << std::setw(LS_WIDTH) << item->d_name;
                        type = "- file";  break;

                    case DT_FIFO:       type = "- fifo";  break;
                    case DT_SOCK:       type = "- socket";  break;
                    default:            type = "- unknown";  break;
                }
                result << ANSI_RESET << type << "\n";
            }
        }
        if (items.empty())
            result << " ";
        closedir(dir);

    } else {
        result << ANSI_COLOR_RED    << "Invalid Path: " + cleanPath + "\n"              << ANSI_RESET;
        result << ANSI_COLOR_RED    << "Error details: " + string(strerror(errno))      << ANSI_RESET;
    }

    return result.str();
 }

bool FileManager::cd(const string& path) {

    if (chdir(path.c_str()) == 0) {
        char temp[4096]{};
        NULLCHECK(getcwd(temp, sizeof(temp)));
        currentPath = string(temp);
        return true;
    } else {
        return false;
    }
}

string FileManager::statFile(const string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) < 0) {
        ostringstream error;
        error << ANSI_COLOR_RED << "Invalid Path: " + path + "\n"                  << ANSI_RESET;
        error << ANSI_COLOR_RED << "Error details: " + string(strerror(errno))     << ANSI_RESET;
        return error.str();
    }

    mode_t mode = info.st_mode;
    string fileMode("unknown");
    switch(mode & S_IFMT) {
        case S_IFREG:   fileMode = "regular file";     break;          
        case S_IFDIR:   fileMode = "directory";        break;      
        case S_IFIFO:   fileMode = "fifo";             break; 
        case S_IFSOCK:  fileMode = "socket";           break;   
        default:
            break;
    }

    string result{};
    result += "File: " + path + "\n";
    result += "Type: " + fileMode + "\n";
    result += "Size: " + to_string(info.st_size) + "\n";
    result += "File Permissions: ";
    result += this->getPermissionsAsString(info.st_mode) + "\n";
    result += "UID: " + to_string(info.st_uid) + "\n";
    result += "GID: " + to_string(info.st_gid) + "\n";

    return result;
}

bool FileManager::makeDirectory(const string& path) {
    //rw everyone && w owner
    if (mkdir(path.c_str(), 0755) == -1) {
        return false; 
    }
    return true;
}

bool FileManager::transfer(const string& localPath, const string& remotePath, const types type) {

    struct stat info;
    if (stat(localPath.c_str(), &info) < 0) {
        return false; 
    }

    if (S_ISDIR(info.st_mode)) {
        DIR *dir = opendir(localPath.c_str());
        if (!dir) {
            return false; 
        }

        msg_header hd;
        hd.type = types::MK_DIR;
        strcpy(hd.path, remotePath.c_str());

        HANDLE_NO_EXIT(write(remote, &hd, sizeof(hd)));
        HANDLE_NO_EXIT(read(remote, &hd, sizeof(hd)));
        if (hd.type == types::ERROR) {
            std::cout << ANSI_COLOR_RED 
                      << "Error creating directory: " << remotePath << '\n' << "Maybe it already exists?\n"
                      << ANSI_RESET;
            return false;
        }

        struct dirent *item;
        while ((item = readdir(dir)) != NULL) {
            string name = item->d_name;
            if (name == "." || name == "..") continue; 

            string fullPath = localPath + "/" + name;
            string remoteFullPath = remotePath + "/" + name;

            if (!transfer(fullPath, remoteFullPath, type)) {
                closedir(dir);
                return false;
            }
        }
        closedir(dir);
    } else {
        pthread_mutex_lock(&mutex);

        FILE *source;
        NULLCHECK_NO_EXIT(source = fopen(localPath.c_str(), "rb"));
        char *chunk = (char *)calloc(CHUNK, 1);
        uint nBytes;

        msg_header hd;
        hd.type         = type;
        hd.content_size = (size_t)info.st_size;
        strcpy(hd.path, remotePath.c_str());

        HANDLE_NO_EXIT(write(remote, &hd, sizeof(hd)));

        uint32_t t = 0;
        while((nBytes = fread(chunk, 1, CHUNK, source)) > 0) {
            t += nBytes;
            HANDLE_NO_EXIT(write(remote, chunk, CHUNK));
        }
        free(chunk);

        pthread_mutex_unlock(&mutex);

        usleep(50000);
        return true;
    }
    return true;
}

bool FileManager::acceptTransfer(const msg_header header) {
    int dest;
    pthread_mutex_lock(&mutex);
    //644 read and write for the owner, and read-only for others
    HANDLE_NO_EXIT(dest = open(header.path, O_RDWR | O_CREAT, 0644));

    uint bytesRead = 0;
    uint totalBytes = 0;
    char chunk[CHUNK]{};

    while (totalBytes + CHUNK <= header.content_size) {
        HANDLE_NO_EXIT(bytesRead = read(remote, chunk, CHUNK));
        totalBytes += bytesRead;
        HANDLE_NO_EXIT(write(dest, chunk, bytesRead));
    }
    if (totalBytes + CHUNK > header.content_size) {
        HANDLE_NO_EXIT(bytesRead = read(remote, chunk, header.content_size - totalBytes));
        totalBytes += (header.content_size - totalBytes);
        HANDLE_NO_EXIT(write(dest, chunk, bytesRead));
    }
    close(dest);

    //pthread_mutex_unlock(&mutex);
    return 1;
}
