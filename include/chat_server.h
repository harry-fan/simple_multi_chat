#ifndef CHAT_SERVER_H
#define CHAT_SERVER_H

#include "network.h"
#include <thread>
#include <vector>
#include <mutex>

class ChatServer {
public:
    ChatServer(int port);
    void run();

private:
    Network network;
    std::vector<int> clientSockets;
    std::mutex clientMutex;

    void handleClient(int clientSocket);
    void broadcastMessage(const std::string& message, int senderSocket);
};

#endif // CHAT_SERVER_H
