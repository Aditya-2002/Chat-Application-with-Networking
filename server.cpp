#include <iostream>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <tchar.h>
#include <thread>
#include <vector>
#include <windows.h>

#pragma comment(lib,"Ws2_32.lib")

// Function to initialize Winsock library
bool Initialize()
{
    WSADATA data;
    return WSAStartup(MAKEWORD(2, 2), &data) == 0;
}

// Function to set console text color
void SetConsoleColor(int color)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

// Function to handle communication with a client
void InteractWithClient(SOCKET clientSocket, std::vector<SOCKET>& clients)
{
    SetConsoleColor(FOREGROUND_GREEN);
    std::cout << "Client Connected" << std::endl;
    char buffer[4096];

    while (true)
    {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            SetConsoleColor(FOREGROUND_RED);
            std::cout << "Client Disconnected" << std::endl;
            break;
        }
        std::string message(buffer, bytesReceived);
        std::cout << "Message from client: " << message << std::endl;

        // Broadcast message to all clients except the sender
        for (auto client : clients)
        {
            if (client != clientSocket)
                send(client, message.c_str(), message.length(), 0);
        }
    }

    // Remove disconnected client from the list
    auto it = std::find(clients.begin(), clients.end(), clientSocket);
    if (it != clients.end()) {
        clients.erase(it);
    }
    closesocket(clientSocket);
}

int main()
{
    // Initialize Winsock
    if (!Initialize())
    {
        std::cout << "Winsock initialization failed" << std::endl;
        return 1;
    }

    // Create listening socket
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET)
    {
        std::cout << "Socket creation failed" << std::endl;
        return 1;
    }

    // Prepare address structure
    int port = 12345;
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    if (InetPton(AF_INET, _T("0.0.0.0"), &serverAddr.sin_addr) != 1)
    {
        std::cout << "Setting address structure failed" << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // Bind socket to IP and port
    if (bind(listenSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cout << "Bind Failed" << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // Start listening for incoming connections
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cout << "Listen Failed" << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // Server started successfully
    SetConsoleColor(FOREGROUND_BLUE);
    std::cout << "Server has started listening on port: " << port << std::endl;

    // Vector to store client sockets
    std::vector<SOCKET> clients;

    // Accept incoming connections and spawn threads to handle them
    while (true)
    {
        SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET)
            std::cout << "Invalid client Socket " << std::endl;

        clients.push_back(clientSocket);
        std::thread t1(InteractWithClient, clientSocket, std::ref(clients));
        t1.detach();
    }

    // Cleanup and exit
    closesocket(listenSocket);
    WSACleanup();
    return 0;
}
