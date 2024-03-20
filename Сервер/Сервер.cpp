#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <ctime>
#include <unordered_map>
#include <ws2tcpip.h> // Для INET_ADDRSTRLEN и inet_ntop
#include <lmcons.h> // Для UNLEN

#pragma comment(lib, "Ws2_32.lib") // Для использования Winsock

#ifndef IPPROTO_TCP
#define IPPROTO_TCP 6
#endif

struct ClientInfo {
    std::string domain;
    std::string machineName;
    std::string ipAddress;
    std::string username;
    time_t lastActivity;
};

std::unordered_map<SOCKET, ClientInfo> connectedClients;

// Функция для отображения данных на рабочем столе
void displayOnDesktop(const char* data) {
    int length = strlen(data) + 1;
    wchar_t* wideData = new wchar_t[length];
    size_t convertedChars = 0;
    mbstowcs_s(&convertedChars, wideData, length, data, _TRUNCATE);

    MessageBox(NULL, wideData, L"Received Data", MB_OK);

    delete[] wideData;
}

// Функция для обновления информации о клиенте
void updateClientInfo(SOCKET clientSocket, const sockaddr_in& clientAddr) {
    ClientInfo& clientInfo = connectedClients[clientSocket];

    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(clientAddr.sin_addr), ip, INET_ADDRSTRLEN);
    clientInfo.ipAddress = ip;

    char hostname[1024];
    gethostname(hostname, 1024);
    clientInfo.machineName = hostname;

    TCHAR username[UNLEN + 1];
    DWORD usernameLen = UNLEN + 1;
    GetUserName(username, &usernameLen);
    clientInfo.username = std::string(username, username + wcslen(username));

    clientInfo.lastActivity = time(nullptr);
}

// Функция для обновления времени последней активности клиента
void updateLastActivity(SOCKET clientSocket) {
    ClientInfo& clientInfo = connectedClients[clientSocket];
    clientInfo.lastActivity = time(nullptr);
}

// Функция для получения времени последней активности клиента
time_t getLastActivity(SOCKET clientSocket) {
    return connectedClients[clientSocket].lastActivity;
}

// Функция для получения информации о всех подключенных клиентах
std::string getAllConnectedClientsInfo() {
    std::string info;
    for (const auto& pair : connectedClients) {
        const ClientInfo& client = pair.second;
        info += "Domain: " + client.domain + "\n";
        info += "Machine Name: " + client.machineName + "\n";
        info += "IP Address: " + client.ipAddress + "\n";
        info += "Username: " + client.username + "\n";

        // Преобразование времени последней активности в строку
        char timeStr[26];
        ctime_s(timeStr, sizeof(timeStr), &client.lastActivity);
        info += "Last Activity: " + std::string(timeStr);

        info += "------------------------\n";
    }
    return info;
}

int main() {
    WSADATA wsaData;
    SOCKET ListenSocket, ClientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    int clientAddrSize = sizeof(clientAddr);
    char recvBuffer[1024];
    int recvSize;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ListenSocket == INVALID_SOCKET) {
        std::cerr << "Error creating socket\n";
        WSACleanup();
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8888);

    if (bind(ListenSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed\n";
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed\n";
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    while (true) {
        ClientSocket = accept(ListenSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);
        if (ClientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed\n";
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }

        updateClientInfo(ClientSocket, clientAddr);

        recvSize = recv(ClientSocket, recvBuffer, sizeof(recvBuffer), 0);
        if (recvSize > 0) {
            recvBuffer[recvSize] = '\0';
            std::cout << "Received data from client: " << recvBuffer << std::endl;

            // Отображение данных на рабочем столе
            displayOnDesktop(recvBuffer);

            // Обновление времени последней активности клиента
            updateLastActivity(ClientSocket);
        }

        std::cout << "Connected clients:\n" << getAllConnectedClientsInfo() << std::endl;

        closesocket(ClientSocket);
    }

    closesocket(ListenSocket);
    WSACleanup();

    return 0;
}
