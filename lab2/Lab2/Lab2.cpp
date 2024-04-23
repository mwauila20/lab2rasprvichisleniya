#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <sstream>
#pragma comment(lib, "ws2_32.lib")



int main() {
    double accumulatedResult = 0.0;
    WSADATA wsaData;
    setlocale(LC_ALL, "en_us");
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    int port;
    std::cout << "Enter server port: ";
    std::cin >> port;

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server listening on port " << port << "\n";

    std::vector<SOCKET> clients;
    fd_set readSet;
    char buffer[1024];
    int bytesRead;

    while (true) {
        FD_ZERO(&readSet);
        FD_SET(serverSocket, &readSet);

        for (const auto& client : clients) {
            FD_SET(client, &readSet);
        }

        int activity = select(0, &readSet, nullptr, nullptr, nullptr);
        if (activity == SOCKET_ERROR) {
            std::cerr << "Select failed\n";
            break;
        }

        if (FD_ISSET(serverSocket, &readSet)) {
            // New connection
            SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
            if (clientSocket == INVALID_SOCKET) {
                std::cerr << "Accept failed\n";
                break;
            }

            clients.push_back(clientSocket);

            char clientIP[INET_ADDRSTRLEN];
            sockaddr_in clientAddr;
            int clientAddrSize = sizeof(clientAddr);
            getpeername(clientSocket, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrSize);
            inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);

            std::cout << "Client " << clients.size() << " connected: " << clientIP << ":" << ntohs(clientAddr.sin_port) << "\n";
        }

        for (auto it = clients.begin(); it != clients.end(); ) {
            SOCKET clientSocket = *it;

            if (FD_ISSET(clientSocket, &readSet)) {
                bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
                if (bytesRead > 0) {
                    buffer[bytesRead] = '\0';
                    std::cout << "Received from client " << std::distance(clients.begin(), it) + 1 << ": " << buffer << "\n";

                    // Handle your logic here based on received data

                    // For example, if received "Task"
                    if (std::string(buffer) == "Task") {
                        double start = 10.0;
                        double end = 20.0;
                        double step = 0.0000001;
                        if (std::distance(clients.begin(), it) + 1 == 1)
                        {
                            start = 10;
                            end = 15;
                        }
                        else {
                            start = 15;
                            end = 20;
                        }
                        // Convert double values to strings
                        std::ostringstream startStr, endStr, stepStr;
                        startStr << start;
                        endStr << end;
                        stepStr << step;

                        std::string params = startStr.str() + " " + endStr.str() + " " + stepStr.str();

                        std::cout << "Sending integral parameters to client " << std::distance(clients.begin(), it) + 1 << ": " << params << "\n";

                        // Send the parameters to the client
                        send(clientSocket, params.c_str(), params.size() + 1, 0);
                    }
                    if (strncmp(buffer, "Result-", 7) == 0) {
                        // Extract the result from the message (skip "Result-" prefix)
                        double result = std::stod(buffer + 7);

                        std::cout << "Received integral result from client " << std::distance(clients.begin(), it) + 1 << ": " << result << "\n";

                        // Accumulate the result received from the client
                        accumulatedResult += result;
                        std::cout << "Accumulated result: " << accumulatedResult << "\n";
                        
                    }
                }
                
                else {
                    // Connection closed by client
                    std::cout << "Client " << std::distance(clients.begin(), it) + 1 << " disconnected\n";
                    closesocket(clientSocket);
                    it = clients.erase(it);
                    continue;
                }
            }

            ++it;
        }
    }

    // Cleanup
    for (const auto& client : clients) {
        closesocket(client);
    }

    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
