#ifndef NETWORK_H
#define NETWORK_H

#include <string>
#include <vector>

class Network {
public:
    Network();
    ~Network();

    bool startServer(int port);
    bool connectToServer(const std::string& ip, int port);
    void sendData(int clientSock, const std::string& data);
    std::string receiveData();
    int getServerSocket();
    int getClientSocket();
    void setClientSocket(int clientSock);
    void setServerSocket(int serverSock);

private:
    int clientSocket;
    int serverSocket;
};

#endif // NETWORK_H
