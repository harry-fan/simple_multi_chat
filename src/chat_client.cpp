#include "chat_client.h"
#include <iostream>
#include <unistd.h>
#include <netinet/in.h>

ChatClient::ChatClient(const std::string& ip, int port) : network() {
    if (!network.connectToServer(ip, port)) {
        std::cerr << "Failed to connect to server" << std::endl;
        exit(EXIT_FAILURE);
    }

    receiveThread = std::thread(&ChatClient::receiveMessages, this);
}

void ChatClient::run() {
    while (true) {
        std::string message;
        std::getline(std::cin, message);
        if (message.empty()) {
            continue;
        }
        network.sendData(network.getClientSocket(), message);
    }
}

void ChatClient::receiveMessages() {
    int32_t bodySize = 0;
    int32_t recvSize = 0;
    std::string recvMessage = "";
    const NetworkData *head = NULL;
    while (true) {
        char buffer[1024] = {0};
        int valread = read(network.getClientSocket(), buffer, 1024);
        if (valread < sizeof(NetworkData)) {
            std::cout << "Received message error!" << network.getClientSocket() << "|" << valread << std::endl;
            continue;
        }
        if (head == NULL) {
            head = reinterpret_cast<const NetworkData *>(buffer);
            bodySize = ntohl(head->packetSize);
            if (valread < sizeof(NetworkData)) {
                std::cout << "Received message error!" << network.getClientSocket() << "|" << valread << std::endl;
                return;
            }
            recvSize += valread - sizeof(NetworkData);
            recvMessage += std::string(buffer + sizeof(NetworkData), recvSize);
        } else {
            recvSize += valread;
            recvMessage += std::string(buffer, valread);
        }
        if (recvSize < bodySize) {
            continue;
        }
        std::cout << recvMessage << std::endl;
        head = NULL;
        recvMessage.clear();
        bodySize = 0, recvSize = 0;
    }
}
