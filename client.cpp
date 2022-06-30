#include "header.h"

// Use for Visual C++
// #pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")
// #pragma comment (lib, "AdvApi32.lib")

int __cdecl main(int argc, char **argv) 
{
    system("CLS");

    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;

    struct addrinfo *result = NULL, *ptr = NULL, hints;
    const char *sendbuf = "this is a test";
    char recvbuf[DEF_BUF_LEN];

    int iResult;
    int iSendResult;
    int recvbuflen = DEF_BUF_LEN;

    // Validate the parameters
    if (argc != 2) {
        printf("usage: %s server-name\n", argv[0]);
        return 1;
    }

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(argv[1], DEF_PORT, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    std::cout << "--------------------\n" <<
    "Royal Restaurant's Menu\n\n";
    char curFood[DEF_BUF_LEN];

    // Print out the menu
    while (true) {
        iResult = recv(ConnectSocket, curFood, recvbuflen, 0);
        curFood[iResult] = '\0';

        if (curFood == "End\0") break;

        std::istringstream iss(curFood);
        std::string item;

        std::cout << std::left;
        while (getline(iss, item, ','))
        {
            std::cout << std::setw(20) << item;
        }
        std::cout << std::endl;

        iSendResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
    }

    // Ask what to order
    std::cout << "\nWhat would you like to order? ";
    std::string order;
    std::getline(std::cin, order);
    iSendResult = send(ConnectSocket, order.c_str(), sizeof(order), 0);

    // Shutdown the connection
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    // Cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}