#include <iostream>
#include <thread>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>

const int PORT = 8080;
const int MAX_MESSAGE_SIZE = 1024;

void handleReceivedMessage(int clientSocket) {
    char buffer[MAX_MESSAGE_SIZE];
    while (true) {
        int bytesReceived = recv(clientSocket, buffer, MAX_MESSAGE_SIZE, 0);
        
        if (bytesReceived <= 0) {
            std::cout << "Server disconnected" << std::endl;
            close(clientSocket);
            break;
        } else {
            buffer[bytesReceived] = '\0'; // Ensure null-termination for string operations
            std::cout << "Received: " << buffer << std::endl;
            std::cout << "Enter message (or 'exit' to quit): ";
            std::cout.flush(); // Flush the output buffer
        }
    }
}
void sendMessage(int clientSocket, const std::string& username) {
    std::string message;
    while (true) {
        std::cout << "Enter message (or 'exit' to quit): ";
        std::getline(std::cin, message);

        if (message == "exit") {
            break;
        }

        std::string fullMessage;
        if (message.substr(0, 5) == "MESG|") {
            // If it's a private message, send as is
            fullMessage = message;
        } else {
            // If it's a broadcast message, construct the message format
            fullMessage = username + " send " + message;
        }

        // Padding the message to reach MAX_MESSAGE_SIZE
        fullMessage.resize(MAX_MESSAGE_SIZE, '\0');

        send(clientSocket, fullMessage.c_str(), fullMessage.size(), 0);
    }
}

int main() {
    int clientSocket;
    struct sockaddr_in serverAddr;

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return -1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Connection failed" << std::endl;
        return -1;
    }

    std::string username;
    std::cout << "Enter your username: ";
    std::getline(std::cin, username);

    // Sending the username to the server after connecting
    send(clientSocket, username.c_str(), username.size(), 0);

    std::thread receivingThread(handleReceivedMessage, clientSocket);
    receivingThread.detach();

    sendMessage(clientSocket, username);

    close(clientSocket);
    return 0;
}
