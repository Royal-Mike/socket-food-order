#undef UNICODE

#include "header.h"

// Use for Visual C++
// #pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

// Save current order to database
void saveOrder(int& iSendResult, SOCKET& ClientSocket, ORDER*& orders, std::string allFood, std::string allQuantity, float priceTotal) {
    int id = 1;
    ORDER* curOrder = orders;

    if (orders == nullptr) {
        orders = new ORDER;
        curOrder = orders;
    }

    else {
        while (curOrder -> next != nullptr) {
            id++;
            curOrder = curOrder -> next;
        }
        id++;
        curOrder -> next = new ORDER;
        curOrder = curOrder -> next;
    }

    curOrder -> id = id;
    curOrder -> food = allFood;
    curOrder -> quantity = allQuantity;
    curOrder -> price = priceTotal;
    curOrder -> status = "Unpaid";
    curOrder -> next = nullptr;

    std::fstream outputOrder;
    outputOrder.open("order.txt", std::ios::out);
    outputOrder << "ID,Food,Quantity,Price,Status\n";

    curOrder = orders;
    while (true) {
        outputOrder << curOrder -> id << ","
        << curOrder -> food << ","
        << curOrder -> quantity << ","
        << curOrder -> price << ","
        << curOrder -> status;
        if (curOrder -> next != nullptr) {
            outputOrder << std::endl;
            curOrder = curOrder -> next;
        }
        else break;
    }
}

// Accept client's payment and update order in database
void acceptPayment(int& iSendResult, SOCKET& ClientSocket, ORDER*& orders) {
    std::string message = "P";
    iSendResult = send(ClientSocket, message.c_str(), sizeof(message), 0);

    ORDER* curOrder = orders;
    while (curOrder -> next != nullptr) {
        curOrder = curOrder -> next;
    }
    curOrder -> status = "Paid";

    std::fstream outputOrder;
    outputOrder.open("order.txt", std::ios::out);
    outputOrder << "ID,Food,Quantity,Price,Status\n";

    curOrder = orders;
    while (true) {
        outputOrder << curOrder -> id << ","
        << curOrder -> food << ","
        << curOrder -> quantity << ","
        << curOrder -> price << ","
        << curOrder -> status;
        if (curOrder -> next != nullptr) {
            outputOrder << std::endl;
            curOrder = curOrder -> next;
        }
        else break;
    }
}

int __cdecl main(void) {
    system("CLS");

    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo *result = NULL;
    struct addrinfo hints;

    int iSendResult;
    const char *sendbuf = "[TEST]";
    char recvbuf[DEF_BUF_LEN];
    int recvbuflen = DEF_BUF_LEN;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("Winsock Error: %d\n", iResult);
        return 1;
    }

    std::cout << "Winsock initialized.\n";

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve server address and port
    iResult = getaddrinfo(NULL, DEF_PORT, &hints, &result);
    if (iResult != 0) {
        printf("Getaddrinfo Error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create listen socket
    ListenSocket = socket(result -> ai_family, result -> ai_socktype, result -> ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("Socket Error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    std::cout << "Socket created.\n";

    // Setup TCP for listen socket
    iResult = bind(ListenSocket, result -> ai_addr, (int)result -> ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("Bind Error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Waiting for connections...\n";

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("Listen Error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    // Accept a client socket
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        printf("Accept Error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to client.\n";

    // Read order file
    std::fstream inputOrder;
    inputOrder.open("order.txt", std::ios::in);
    ORDER* orders = nullptr;
    ORDER* oneOrder = orders;
    int countMenu = -1;

    // Put order data into linked list
    while (!inputOrder.eof()) {
        std::string dataOrder;
        std::getline(inputOrder, dataOrder);

        countMenu++;
        if (countMenu == 0) continue;

        std::istringstream iss(dataOrder);
        std::string item;
        std::vector<std::string> seglist;

        while (std::getline(iss, item, ',')) {
            seglist.push_back(item);
        }

        if (oneOrder == nullptr) {
            orders = new ORDER;
            oneOrder = orders;
        }
        else {
            oneOrder -> next = new ORDER;
            oneOrder = oneOrder -> next;
        }

        oneOrder -> id = std::atoi(seglist[0].c_str());
        oneOrder -> food = seglist[1];
        oneOrder -> quantity = seglist[2];
        oneOrder -> price = std::atof(seglist[3].c_str());
        oneOrder -> status = seglist[4];
        oneOrder -> next = nullptr;
    }

    // Read menu file
    std::fstream inputMenu;
    inputMenu.open("menu.txt", std::ios::in);
    FOOD* menu = new FOOD;
    FOOD* food = menu;
    int count = -1;

    // Send menu data to client
    while (true) {
        std::string curFood;
        std::getline(inputMenu, curFood);
        iSendResult = send(ClientSocket, curFood.c_str(), sizeof(curFood), 0);
        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);

        if (inputMenu.eof()) {
            food -> next = nullptr;
            break;
        }

        count++;
        if (count == 0) continue;

        std::istringstream iss(curFood);
        std::string item;
        std::vector<std::string> seglist;

        while (std::getline(iss, item, ',')) {
            seglist.push_back(item);
        }

        food -> id = std::atoi(seglist[0].c_str());
        food -> name = seglist[1];
        food -> price = std::atof(seglist[2].c_str());
        food -> next = new FOOD;
        food = food -> next;
    }

    ORDER* curOrder = nullptr;
    ORDER* curOrderFood = curOrder;

    // Receive and validate client's order
    while (true) {
        std::string order;
        char buffer[DEF_BUF_LEN];
        iResult = recv(ClientSocket, buffer, recvbuflen, 0);
        buffer[iResult] = '\0';
        order = buffer;

        if (order == "p") {
            curOrderFood = curOrder;
            std::string delim = "|";
            std::string allFood = "";
            std::string allQuantity = "";
            float priceTotal = 0;

            while (true) {
                allFood += curOrderFood -> food;
                allQuantity += curOrderFood -> quantity;
                priceTotal += curOrderFood -> price * std::atoi(curOrderFood -> quantity.c_str());
                curOrderFood = curOrderFood -> next;
                if (curOrderFood != nullptr) {
                    allFood += delim;
                    allQuantity += delim;
                }
                else break;
            }

            saveOrder(iSendResult, ClientSocket, orders, allFood, allQuantity, priceTotal);
            std::string message = std::to_string(priceTotal);
            iSendResult = send(ClientSocket, message.c_str(), sizeof(message), 0);
            break;
        }

        std::istringstream iss(order);
        std::string item;
        std::vector<std::string> seglist;

        while (std::getline(iss, item, ',')) {
            seglist.push_back(item);
        }

        bool check = false;
        food = menu;
        while (food != nullptr) {
            if (food -> name == seglist[0]) {
                check = true;
                break;
            }
            food = food -> next;
        }

        if (seglist.size() != 2) check = false;

        if (check) {
            for (int i = 0; i < seglist[1].size(); i++) {
                if (isdigit(seglist[1][i])) check = true;
                else check = false;
            }

            if (seglist[1][0] == '0' || seglist[1][0] == '-') check = false;
        }

        if (!check) {
            std::string message = "I";
            iSendResult = send(ClientSocket, message.c_str(), sizeof(message), 0);
        }

        else {
            if (curOrderFood == nullptr) {
                curOrder = new ORDER;
                curOrderFood = curOrder;                
            }
            else {
                curOrderFood -> next = new ORDER;
                curOrderFood = curOrderFood -> next;
            }

            curOrderFood -> food = seglist[0];
            curOrderFood -> price = food -> price;
            curOrderFood -> quantity = seglist[1];
            curOrderFood -> next = nullptr;

            std::string message = "O";
            iSendResult = send(ClientSocket, message.c_str(), sizeof(message), 0);
        }
    }

    // Receive and validate client's payment
    while (true) {
        std::string payment;
        char buffer[DEF_BUF_LEN];
        iResult = recv(ClientSocket, buffer, recvbuflen, 0);
        buffer[iResult] = '\0';
        payment = buffer;

        if (payment == "c") {
            acceptPayment(iSendResult, ClientSocket, orders);
            break;
        }

        else {
            bool check = true;
            if (payment.size() != 10) check = false;

            if (check)
            for (int i = 0; i < payment.size(); i++) {
                if (!isdigit(payment[i])) {
                    check = false;
                    break;
                }
            }

            if (check) {
                acceptPayment(iSendResult, ClientSocket, orders);
                break;
            }

            else {
                std::string message = "I";
                iSendResult = send(ClientSocket, message.c_str(), sizeof(message), 0);
            }
        }
    }

    // Close listen socket
    closesocket(ListenSocket);

    // Shutdown connection
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("Shutdown Error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    // Clean up
    closesocket(ClientSocket);
    WSACleanup();

    // Delete menu linked list
    food = menu;
    while (food != nullptr) {
        FOOD* temp = food;
        food = food -> next;
        delete temp;
    }
    delete menu;

    // Delete order linked list
    curOrderFood = curOrder;
    while (curOrderFood != nullptr) {
        ORDER* temp = curOrderFood;
        curOrderFood = curOrderFood -> next;
        delete temp;
    }
    delete curOrder;

    return 0;
}