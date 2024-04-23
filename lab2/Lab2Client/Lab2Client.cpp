#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <sstream>
#include <cmath>
#pragma comment(lib, "ws2_32.lib")

double Function(double x) {
    return 1.0 / log(x);
}

// Функция для расчета интеграла
double CalculateIntegral(double start, double end, double step) {
    double sum = 0.0;
    for (double x = start; x < end; x += step) {
        sum += Function(x) * step;
    }
    return sum;
}

int main() {
    WSADATA wsaData;
    setlocale(LC_ALL, "en_us");
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    std::string serverIP;
    std::cout << "Enter server IP: ";
    std::cin >> serverIP;

    int port;
    std::cout << "Enter server port: ";
    std::cin >> port;

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, serverIP.c_str(), &(serverAddr.sin_addr));
    serverAddr.sin_port = htons(port);

    if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connection failed\n";
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to server\n";
    while (true) {
        std::string message;
        std::cout << "Enter message (or 'exit' to quit): ";
        std::getline(std::cin, message);

        if (message == "exit") {
            break;
        }

        send(clientSocket, message.c_str(), message.size(), 0);

        char buffer[1024];

        if (message == "Task") {
            double start, end, step;
            char paramsBuffer[1024];

            // Receive the parameters from the server
            recv(clientSocket, paramsBuffer, sizeof(paramsBuffer), 0);

            // Parse the received string back to double values
            std::istringstream paramsStream(paramsBuffer);
            paramsStream >> start >> end >> step;

            std::cout << "Received integral parameters from server: start=" << start << ", end=" << end << ", step=" << step << "\n";

            double result = CalculateIntegral(start, end, step);

            // Convert the result to a string before sending
            std::ostringstream resultStr;
            resultStr << "Result-" << result;
            
            send(clientSocket, resultStr.str().c_str(), resultStr.str().size() + 1, 0);
        }
    }

    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
