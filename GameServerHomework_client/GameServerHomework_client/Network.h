#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32")

#include <WinSock2.h>
#include <iostream>
#include <stdlib.h>

using namespace std;

#define SERVERIP "192.168.122.208"
#define SERVERPORT 4000
#define BUFSIZE 512

void err_quit(const char* msg);

void err_display(const char* msg);

int recvn(SOCKET s, char* buf, int len, int flags);

//int netInit(int argc, char* argv[]);

int netInit();
