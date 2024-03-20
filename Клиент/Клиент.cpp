#define _WINSOCK_DEPRECATED_NO_WARNINGS // Отключаем предупреждения об устаревших функциях
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <string>

#pragma comment(lib, "Ws2_32.lib") // Для использования Winsock

// Функция для отправки данных на сервер
void sendData(const char* serverIP, int serverPort, const char* data) {
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct sockaddr_in serverAddr;

    // Инициализация Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return;
    }

    // Создание сокета
    ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ConnectSocket == INVALID_SOCKET) {
        std::cerr << "Error creating socket\n";
        WSACleanup();
        return;
    }

    // Установка параметров сервера
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    serverAddr.sin_addr.s_addr = inet_addr(serverIP);

    // Подключение к серверу
    if (connect(ConnectSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Error connecting to server\n";
        closesocket(ConnectSocket);
        WSACleanup();
        return;
    }

    // Отправка данных на сервер
    send(ConnectSocket, data, strlen(data), 0);

    // Закрытие сокета и очистка Winsock
    closesocket(ConnectSocket);
    WSACleanup();
}


// Функция для получения активного окна
std::string getActiveWindow() {
    char windowTitle[256];
    HWND hwnd = GetForegroundWindow();
    GetWindowText(hwnd, windowTitle, sizeof(windowTitle));
    return std::string(windowTitle);
}

int main() {
    const char* serverIP = "127.0.0.1"; // IP адрес сервера
    int serverPort = 8888; // Порт сервера

    // Основной цикл программы
    while (true) {
        // Получаем активное окно
        std::string activeWindow = getActiveWindow();

        // Преобразуем строку в формат wchar_t
        const wchar_t* wideData = reinterpret_cast<const wchar_t*>(activeWindow.c_str());

        // Отправляем данные на сервер
        sendData(serverIP, serverPort, activeWindow.c_str());

        // Задержка перед следующей итерацией (например, 10 секунд)
        Sleep(10000);
    }

    return 0;
}
