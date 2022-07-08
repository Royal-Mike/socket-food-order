#include "header.h"

// Use for Visual C++
// #pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")
// #pragma comment (lib, "AdvApi32.lib")

// Ask client for order
void askOrder(int& iSendResult, SOCKET ConnectSocket) {
    std::cout << "\n" << HORI_LINE << "\n" <<
    "What would you like to order? ";
    std::string order;
    std::getline(std::cin, order);
    unsigned int length = strlen(order.c_str());
    if (length > DEF_BUF_LEN) length = DEF_BUF_LEN;
    iSendResult = send(ConnectSocket, order.c_str(), length, 0);
}

// Ask client for payment
void askPayment(int& iSendResult, SOCKET ConnectSocket) {
    std::cout << "\n" << HORI_LINE << "\n" <<
    "Enter c to pay by cash, or a 10-digit bank account number to pay with your account: ";
    std::string payment;
    std::getline(std::cin, payment);
    unsigned int length = strlen(payment.c_str());
    if (length > DEF_BUF_LEN) length = DEF_BUF_LEN;
    iSendResult = send(ConnectSocket, payment.c_str(), length, 0);
}

int __cdecl main(int argc, char **argv) {
    system("CLS");

    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;

    struct addrinfo *result = NULL, *ptr = NULL, hints;
    const char *sendbuf = "[TEST]";
    char recvbuf[DEF_BUF_LEN];

    int iResult;
    int iSendResult;
    int recvbuflen = DEF_BUF_LEN;

    // Arguments validation
    if (argc != 2) {
        printf("Usage: %s server-name\n", argv[0]);
        return 1;
    }

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("Winsock Error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve server address and port
    iResult = getaddrinfo(argv[1], DEF_PORT, &hints, &result);
    if (iResult != 0) {
        printf("Getaddrinfo Error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Loop until succeed in connecting to an address
    for (ptr = result; ptr != NULL; ptr = ptr -> ai_next) {
        // Create connect socket
        ConnectSocket = socket(ptr -> ai_family, ptr -> ai_socktype, ptr -> ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("Socket Error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server
        iResult = connect(ConnectSocket, ptr -> ai_addr, (int)ptr -> ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }

        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Could not connect to server.\n");
        WSACleanup();
        return 1;
    }

    std::cout << HORI_LINE << "\n" <<
    "Royal Restaurant's Menu\n\n";

    // Print out the menu
    while (true) {
        char curFood[DEF_BUF_LEN];
        iResult = recv(ConnectSocket, curFood, recvbuflen, 0);
        iSendResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
        curFood[iResult] = '\0';

        if (curFood[0] == 'E') break;

        std::istringstream iss(curFood);
        std::string item;

        std::cout << std::left;
        while (getline(iss, item, ','))
        {
            std::cout << std::setw(15) << item;
        }
        std::cout << std::endl;
    }

    // Ask what to order
    askOrder(iSendResult, ConnectSocket);

    // Receive server's response on order
    while (true) {
        std::string message;
        char buffer[DEF_BUF_LEN];
        iResult = recv(ConnectSocket, buffer, recvbuflen, 0);
        buffer[iResult] = '\0';
        message = buffer;

        if (buffer[0] == 'I') {
            std::cout << "Invalid order. Please try again.\n";
            askOrder(iSendResult, ConnectSocket);
        }

        else if (buffer[0] == 'O') {
            std::string choice;
            std::cout << "Food ordered. Enter p to go to payment, or anything else to order more food: ";
            std::getline(std::cin, choice);

            if (choice == "p") {
                unsigned int length = strlen(choice.c_str());
                if (length > DEF_BUF_LEN) length = DEF_BUF_LEN;
                iSendResult = send(ConnectSocket, choice.c_str(), length, 0);
            }
            else askOrder(iSendResult, ConnectSocket);
        }

        else {
            std::cout << "\n" << HORI_LINE << "\n" << "Total price is " << message << ".\n";
            askPayment(iSendResult, ConnectSocket);
            break;
        }
    }

    // Receive server's response on payment
    while (true) {
        std::string message;
        char buffer[DEF_BUF_LEN];
        iResult = recv(ConnectSocket, buffer, recvbuflen, 0);
        buffer[iResult] = '\0';
        message = buffer;

        if (buffer[0] == 'I') {
            std::cout << "Invalid input. Please try again.\n";
            askPayment(iSendResult, ConnectSocket);
        }

        else {
            std::cout << "\n" << HORI_LINE << "\n" << "Payment processed. Thank you for ordering!";
            break;
        }
    }

    // Shutdown connection
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    // Clean up
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}