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

string FileManager::getCurrentPath() {
    return currentPath;
}

string FileManager::ls(const string& path) {
    struct dirent *item;
    string result;
    string cleanPath = path;

    cleanPath.erase(remove_if(cleanPath.begin(), cleanPath.end(), [](char c) {
        return isspace(c);
    }), cleanPath.end());

    DIR *dir = opendir(cleanPath.c_str());
    if (dir != nullptr) {
        while ((item = readdir(dir)) != nullptr) {
            if (strcmp(item->d_name, ".") != 0 && strcmp(item->d_name, "..") != 0) {
                result += item->d_name;
                result += "\n";
            }
        }
        result.pop_back();
        closedir(dir);
    } else {
        result = "Invalid Path: " + cleanPath + "\n";
        result += "Error details: " + string(strerror(errno));
    }

    return result;
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
        string error;
        error = "Invalid Path: " + path + "\n";
        error += "Error details: " + string(strerror(errno));
    }

    string result{};
    result += "File: " + path + "\n";
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

bool FileManager::upload(const string& localPath, const string& remotePath) {

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
        hd.type         = types::MK_DIR;
        strcpy(hd.data.path, remotePath.c_str());
        HANDLE_NO_EXIT(write(remote, &hd, sizeof(hd)));
        HANDLE_NO_EXIT(read(remote, &hd, sizeof(hd)));
        if (hd.type == types::ERROR) {
            return 0;
        }

        struct dirent *item;
        while ((item = readdir(dir)) != NULL) {
            string name = item->d_name;
            if (name == "." || name == "..") continue; 

            string fullPath = localPath + "/" + name;
            string remoteFullPath = remotePath + "/" + name;

            if (!upload(fullPath, remoteFullPath)) {
                closedir(dir);
                return false;
            }
        }
        closedir(dir);
    } else {
        FILE *source;
        NULLCHECK(source = fopen(localPath.c_str(), "rb"));
        vector<char> chunk(CHUNK);
        uint nBytes;

        struct stat info;
        if (stat(localPath.c_str(), &info) < 0) {
            return false;
        }

        msg_header hd;
        hd.type         = types::UPLOAD;
        hd.content_size = (size_t)info.st_size;
        strcpy(hd.data.path, remotePath.c_str());
        HANDLE_NO_EXIT(write(remote, &hd, sizeof(hd)));

        while((nBytes = fread(chunk.data(), 1, CHUNK, source)) > 0) {
            HANDLE_NO_EXIT(write(remote, chunk.data(), CHUNK));
        }
    }

    return true;

}

bool FileManager::acceptUpload(const msg_header header) {
    int dest;

    //644 read and write for the owner, and read-only for others
    HANDLE_NO_EXIT(dest = open(header.data.path, O_RDWR | O_CREAT, 0644));

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
    return 1;
}


bool FileManager::download(const string& remote, const string& local) {
    return 0;
}