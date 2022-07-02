#ifndef HEADER_SOCKET
#define HEADER_SOCKET

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>

#define DEF_BUF_LEN 4096
#define DEF_PORT "30000"

struct FOOD {
    int id;
    std::string name;
    float price;
    FOOD* next;
};

#endif