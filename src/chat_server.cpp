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
    int32_t bodySize = 0;
    int32_t recvSize = 0;
    std::string recvMessage = "";
    const NetworkData *head = NULL;
    while (true) {
        char buffer[1024] = {0};
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
            if (head == NULL) {
                head = reinterpret_cast<const NetworkData *>(buffer);
                bodySize = ntohl(head->packetSize);
                if (bytesRead < sizeof(NetworkData)) {
                    std::cout << "Received message error!" << clientSocket << "|" << bytesRead << std::endl;
                    return;
                }
                recvSize += bytesRead - sizeof(NetworkData);
                recvMessage += std::string(buffer + sizeof(NetworkData), recvSize);
            } else {
                recvSize += bytesRead;
                recvMessage += std::string(buffer, bytesRead);
            }
            if (recvSize < bodySize) {
                continue;
            }
            std::cout << "Received message from: " << clientSocket << "|message: " << recvMessage << std::endl;
            std::string sendMessage = "Message from:" + std::to_string(clientSocket) + "|" + recvMessage;
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
