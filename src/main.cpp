#include "chat_server.h"
#include "chat_client.h"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <mode> <port/ip>" << std::endl;
        std::cerr << "Mode: server or client" << std::endl;
        return EXIT_FAILURE;
    }

    std::string mode = argv[1];
    std::string param = argv[2];

    if (mode == "server") {
        int port = std::stoi(param);
        ChatServer server(port);
        server.run();
    } else if (mode == "client") {
        int port = std::stoi(param.substr(param.find(':') + 1));
        std::string ip = param.substr(0, param.find(':'));
        ChatClient client(ip, port);
        client.run();
    } else {
        std::cerr << "Invalid mode. Use 'server' or 'client'." << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
