#include<iostream>
#include<WinSock2.h>
#include<WS2tcpip.h>
#include<thread>
#include <windows.h>

using namespace std;

#pragma comment(lib,"ws2_32.lib")

// Function to initialize Winsock
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

// Function to send messages
void SendMsg(SOCKET s)
{
    SetConsoleColor(FOREGROUND_BLUE); // Set console text color to blue
    cout << "Enter your chat name: ";
    string name;
    getline(cin, name);
    string message;
    while (true)
    {
        getline(cin, message);
        string msg = name + " : " + message;
        int bytesent = send(s, msg.c_str(), msg.length(), 0);
        if (bytesent == SOCKET_ERROR)
        {
            SetConsoleColor(FOREGROUND_RED); // Set console text color to red
            cout << "Error in sending message" << endl;
            break;
        }
        if (message == "quit")
        {
            SetConsoleColor(FOREGROUND_RED);
            cout << "Stopping the application" << endl;
            break;
        }
    }
    closesocket(s);
    WSACleanup();
}

// Function to receive messages
void ReceiveMsg(SOCKET s)
{
    char buffer[4096];
    int recvlength;
    string msg = "";
    while (true)
    {
        recvlength = recv(s, buffer, sizeof(buffer), 0);
        if (recvlength <= 0)
        {
            SetConsoleColor(FOREGROUND_RED);
            cout << "Disconnected from the server" << endl;
            break;
        }
        else {
            SetConsoleColor(FOREGROUND_GREEN); // Set console text color to green
            msg = string(buffer, recvlength);
            cout << msg << endl;
        }
    }
    closesocket(s);
    WSACleanup();
}

int main()
{
    if (!Initialize()) {
        SetConsoleColor(FOREGROUND_RED); // Set console text color to red
        cout << "Initialize Socket Failed" << endl;
        return 1;
    }

    SOCKET s;
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) {
        SetConsoleColor(FOREGROUND_RED);
        cout << "Invalid Socket created" << endl;
        return 1;
    }
    int port = 12345;
    string serveraddress = "127.0.0.1";
    sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);

    // Setting local host address
    inet_pton(AF_INET, serveraddress.c_str(), &(serveraddr.sin_addr));

    if (connect(s, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr)) == SOCKET_ERROR) {
        SetConsoleColor(FOREGROUND_RED);
        cout << "Not able to connect to server" << endl;
        closesocket(s);
        WSACleanup();
        return 1;
    }

    SetConsoleColor(FOREGROUND_GREEN);
    cout << "Successfully connected to server" << endl;

    // Start sender and receiver threads
    thread senderThread(SendMsg, s);
    thread receiverThread(ReceiveMsg, s);

    // Wait for threads to finish
    senderThread.join();
    receiverThread.join();

    WSACleanup();
    return 0;
}
