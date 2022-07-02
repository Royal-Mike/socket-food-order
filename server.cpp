#undef UNICODE

#include "header.h"

// Use for Visual C++
// #pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

int __cdecl main(void) 
{
    system("CLS");

    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo *result = NULL;
    struct addrinfo hints;

    int iSendResult;
    const char *sendbuf = "ANY";
    char recvbuf[DEF_BUF_LEN];
    int recvbuflen = DEF_BUF_LEN;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    std::cout << "Winsock initialized.\n";

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEF_PORT, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for connecting to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    std::cout << "Socket created.\n";

    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Waiting for connections...\n";

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    // Accept a client socket
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to client.\n";

    std::fstream input;
    input.open("menu.txt", std::ios::in);

    // Send menu data to client
    while (true) {
        std::string curFood;
        std::getline(input, curFood);
        iSendResult = send(ClientSocket, curFood.c_str(), sizeof(curFood), 0);
        if (input.eof()) break;
        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
    }

    // Receive client's order
    do {
        std::string order;
        char buffer[DEF_BUF_LEN];
        iResult = recv(ClientSocket, buffer, recvbuflen, 0);
        buffer[iResult] = '\0';
        order = buffer;
        std::cout << order;
        // std::istringstream iss(order);
        // std::string item;
    } while (iResult > 0);

    // No longer need listen socket
    closesocket(ListenSocket);

    // Shutdown the connection
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    // Cleanup
    closesocket(ClientSocket);
    WSACleanup();

    return 0;
}