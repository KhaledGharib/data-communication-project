#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

const int PORT = 8080;
const int MAX_CLIENTS = 5;
const int MAX_MESSAGE_SIZE = 1024;

struct ClientInfo {
    int socket;
    std::string name;
};

std::vector<ClientInfo> clients;
std::mutex clientsMutex;

void handleClient(int clientSocket) {
    char buffer[MAX_MESSAGE_SIZE];
    int bytesReceived = recv(clientSocket, buffer, MAX_MESSAGE_SIZE, 0);
    if (bytesReceived <= 0) {
        std::cout << "Client disconnected" << std::endl;
        close(clientSocket);
        return;
    }

    std::string username(buffer, bytesReceived);
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        clients.push_back({clientSocket, username});
        std::cout << "New user connected: " << username << std::endl;
        std::cout << "Active clients: ";
        for (const auto& client : clients) {
            std::cout << client.name << " ";
        }
        std::cout << std::endl;
    }

    while (true) {
        bytesReceived = recv(clientSocket, buffer, MAX_MESSAGE_SIZE, 0);
        
        if (bytesReceived <= 0) {
            // Handle client disconnection, remove from the list, and inform other clients (GONE command)
            {
                std::lock_guard<std::mutex> lock(clientsMutex);
                clients.erase(std::remove_if(clients.begin(), clients.end(),
                    [clientSocket](const ClientInfo& client) {
                        return client.socket == clientSocket;
                    }),
                    clients.end()
                );
                std::cout << "Client disconnected" << std::endl;
                std::cout << "Active clients: ";
                for (const auto& client : clients) {
                    std::cout << client.name << " ";
                }
                std::cout << std::endl;
            }
            break;
        } else {
            std::string receivedMessage(buffer, bytesReceived);

            if (receivedMessage.substr(0, 5) == "MESG|") {
                size_t pos = receivedMessage.find('|', 5);
                if (pos != std::string::npos) {
                    std::string receiverName = receivedMessage.substr(5, pos - 5);
                    std::string messageContent = receivedMessage.substr(pos + 1);

                    // Find the intended recipient
                    std::lock_guard<std::mutex> lock(clientsMutex);
                    for (const auto& client : clients) {
                        if (client.name == receiverName) {
                            send(client.socket, messageContent.c_str(), messageContent.size(), 0);
                            break;
                        }
                    }
                }
            } else {
                // Broadcast the message to all connected clients (except the sender)
                std::lock_guard<std::mutex> lock(clientsMutex);
                for (const auto& client : clients) {
                    if (client.socket != clientSocket) {
                        send(client.socket, buffer, bytesReceived, 0);
                    }
                }
            }
        }
    }
}

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return -1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Binding failed" << std::endl;
        return -1;
    }

    if (listen(serverSocket, MAX_CLIENTS) < 0) {
        std::cerr << "Listening failed" << std::endl;
        return -1;
    }

    std::cout << "Server started. Listening for incoming connections..." << std::endl;

    while (true) {
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (clientSocket < 0) {
            std::cerr << "Error accepting connection" << std::endl;
            continue;
        }

        std::thread clientThread(handleClient, clientSocket);
        clientThread.detach();
    }

    close(serverSocket);
    return 0;
}
