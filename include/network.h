#ifndef NETWORK_H
#define NETWORK_H

#include <string>
#include <vector>

struct NetworkData {
    int32_t packetSize;
    // char content[0]
};

class Network {
public:
    Network();
    ~Network();

    bool startServer(int port);
    bool connectToServer(const std::string& ip, int port);
    void sendData(int clientSock, const std::string& data);
    int getServerSocket();
    int getClientSocket();
    void setClientSocket(int clientSock);
    void setServerSocket(int serverSock);

private:
    int clientSocket;
    int serverSocket;
};

#endif // NETWORK_H
