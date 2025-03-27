cmake_minimum_required(VERSION 3.1)
project(chat_server)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)

add_executable(chat_server src/main.cpp src/chat_server.cpp src/network.cpp
        src/chat_client.cpp)

target_link_libraries(chat_server PRIVATE Threads::Threads)