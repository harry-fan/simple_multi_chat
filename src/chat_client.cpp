#include "chat_client.h"
#include <iostream>

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
        network.sendData(network.getClientSocket(), message);
    }
}

void ChatClient::receiveMessages() {
    while (true) {
        std::string message = network.receiveData();
        if (message.empty()) {
            break;
        }
        std::cout << "Received: " << message << std::endl;
    }
}
