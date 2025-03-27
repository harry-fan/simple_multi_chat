#ifndef CHAT_CLIENT_H
#define CHAT_CLIENT_H

#include "network.h"
#include <thread>

class ChatClient {
public:
    ChatClient(const std::string& ip, int port);
    void run();

private:
    Network network;
    std::thread receiveThread;

    void receiveMessages();
};

#endif // CHAT_CLIENT_H
