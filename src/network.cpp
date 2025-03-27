#include "network.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

Network::Network() : serverSocket(-1), clientSocket(-1) {}

Network::~Network() {
    if (serverSocket != -1) close(serverSocket);
    if (clientSocket != -1) close(clientSocket);
}

bool Network::startServer(int port) {
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Socket creation failed");
        return false;
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // 设置 SO_REUSEADDR 选项
    int reuse = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    if (bind(serverSocket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        return false;
    }

    if (listen(serverSocket, 3) < 0) {
        perror("Listen failed");
        return false;
    }

    std::cout << "Server listening on port " << port << std::endl;
    return true;
}

bool Network::connectToServer(const std::string& ip, int port) {
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        perror("Socket creation failed");
        return false;
    }

    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return false;
    }

    if (connect(clientSocket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        return false;
    }

    return true;
}

void Network::sendData(int clientSock, const std::string& data) {
    send(clientSock, data.c_str(), data.size(), 0);
}

std::string Network::receiveData() {
    char buffer[1024] = {0};
    int valread = read(clientSocket, buffer, 1024);
    return std::string(buffer, valread);
}

int Network::getServerSocket() {
    return serverSocket;
}

int Network::getClientSocket() {
    return clientSocket;
}

void Network::setClientSocket(int clientSock) {
    clientSocket = clientSock;
}

void Network::setServerSocket(int serverSock) {
    serverSocket = serverSock;
}
