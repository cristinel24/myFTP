#include "utils.h"

void send_payload(int fd, types type, const char* msg, const char* username, const char* path) {
    msg_header header;
    header.type = type;
    msg != nullptr ? header.content_size = strlen(msg) :  header.content_size = 0;
    if (username != nullptr)
        strcpy(header.username, username);

    if (path != nullptr)
        strcpy(header.path, path);

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

RSA* generateKeyPair() {
    RSA* keyPair = RSA_new();
    BIGNUM* exponent = BN_new();

    // exponent set to RSA_F4 = 65537
    BN_set_word(exponent, RSA_F4);
    RSA_generate_key_ex(keyPair, RSA_LENGTH, exponent, nullptr);

    BN_free(exponent);
    return keyPair;
}
std::string encrypt(const char* data, RSA* publicKey) {
    int size = RSA_size(publicKey);
    char* enData = new unsigned char[size];

    int result = -1;
    HANDLE_NO_EXIT(result = RSA_public_encrypt(strlen(data), data, enData, publicKey, RSA_PKCS1_PADDING));

    std::string encrypted(enData, result);
    delete[] enData;

    return encrypted;
}
std::string decrypt(const char* data, RSA* privateKey) {
    int size = RSA_size(publicKey);
    char* deData = new unsigned char[size];

    int result = -1;
    HANDLE_NO_EXIT(result = RSA_private_decrypt(strlen(data), data, deData, privateKey, RSA_PKCS1_PADDING));

    std::string decrypted(deData, result);
    delete[] deData;

    return decrypted;
} 