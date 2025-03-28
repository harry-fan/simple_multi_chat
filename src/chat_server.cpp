#include "chat_server.h"
#include <iostream>
#include <netinet/in.h>
#include <algorithm>
#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>

ChatServer::ChatServer(int port) : network() {
    if (!network.startServer(port)) {
        std::cerr << "Failed to start server" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void ChatServer::run() {
    int epollFd = epoll_create1(0);
    if (epollFd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = network.getServerSocket();
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, network.getServerSocket(), &event) == -1) {
        perror("epoll_ctl");
        exit(EXIT_FAILURE);
    }

    struct epoll_event events[MAX_EVENTS];
    while (true) {
        int numEvents = epoll_wait(epollFd, events, MAX_EVENTS, -1);
        if (numEvents == -1) {
            perror("epoll_wait");
            continue;
        }

        for (int i = 0; i < numEvents; ++i) {
            if (events[i].data.fd == network.getServerSocket()) {
                sockaddr_in clientAddress;
                socklen_t clientAddressLength = sizeof(clientAddress);
                int clientSocket = accept(network.getServerSocket(), (struct sockaddr *)&clientAddress, &clientAddressLength);
                if (clientSocket < 0) {
                    perror("Accept failed");
                    continue;
                }

                fcntl(clientSocket, F_SETFL, O_NONBLOCK);

                event.events = EPOLLIN | EPOLLET;
                event.data.fd = clientSocket;
                if (epoll_ctl(epollFd, EPOLL_CTL_ADD, clientSocket, &event) == -1) {
                    perror("epoll_ctl");
                    close(clientSocket);
                    continue;
                }

                std::lock_guard<std::mutex> lock(clientMutex);
                clientSockets.push_back(clientSocket);
                std::cout << "Client connected: " << clientSocket << std::endl;
            } else {
                handleClient(events[i].data.fd);
            }
        }
    }
    close(epollFd);
}

void ChatServer::handleClient(int clientSocket) {
    char buffer[1024];
    while (true) {
        ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            if (bytesRead == 0) {
                std::cout << "Client disconnected: " << clientSocket << std::endl;
            } else {
                std::cout << "recv bytes error!" << bytesRead << std::endl;
            }
            std::lock_guard<std::mutex> lock(clientMutex);
            auto it = std::remove(clientSockets.begin(), clientSockets.end(), clientSocket);
            if (it != clientSockets.end()) {
                clientSockets.erase(it);
                ::close(clientSocket);
            }
            break;
        } else {
            std::string message(buffer, bytesRead);
            std::cout << "Received message from: " << clientSocket << "|message: " << message << std::endl;
            std::string sendMessage = "Message from:" + std::to_string(clientSocket) + "|" + message;
            broadcastMessage(sendMessage, clientSocket);
            break;
        }
    }
}

void ChatServer::broadcastMessage(const std::string& message, int senderSocket) {
    std::lock_guard<std::mutex> lock(clientMutex);
    for (int clientSocket : clientSockets) {
        network.sendData(clientSocket, message);
    }
}
