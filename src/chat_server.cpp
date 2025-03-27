#include "chat_server.h"
#include <iostream>
#include <netinet/in.h>
#include <algorithm>
#include <unistd.h>

ChatServer::ChatServer(int port) : network() {
    if (!network.startServer(port)) {
        std::cerr << "Failed to start server" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void ChatServer::run() {
    while (true) {
        sockaddr_in clientAddress;
        socklen_t clientAddressLength = sizeof(clientAddress);
        int clientSocket = accept(network.getServerSocket(), (struct sockaddr *)&clientAddress, &clientAddressLength);
        if (clientSocket < 0) {
            perror("Accept failed");
            continue;
        }
        std::cout << "Client connected: " << clientSocket << std::endl;
        std::lock_guard<std::mutex> lock(clientMutex);
        clientSockets.push_back(clientSocket);

        std::thread clientThread(&ChatServer::handleClient, this, clientSocket);
        clientThread.detach();
    }
}

void ChatServer::handleClient(int clientSocket) {
    network.setClientSocket(clientSocket);
    while (true) {
        std::string message = network.receiveData();
        if (message.empty()) {
            std::lock_guard<std::mutex> lock(clientMutex);
            auto it = std::remove(clientSockets.begin(), clientSockets.end(), clientSocket);
            if (it != clientSockets.end()) {
                clientSockets.erase(it, clientSockets.end());
                ::close(clientSocket);
            }
            break;
        } else {
            std::cout << "Received message from: " << clientSocket << "|message: " << message << std::endl;
        }
        broadcastMessage(message, clientSocket);
    }
}

void ChatServer::broadcastMessage(const std::string& message, int senderSocket) {
    std::lock_guard<std::mutex> lock(clientMutex);
    for (int clientSocket : clientSockets) {
        network.sendData(clientSocket, message);
    }
}
