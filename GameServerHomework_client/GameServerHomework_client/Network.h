#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32")

#include <WS2tcpip.h>
#include <iostream>
#include <stdlib.h>
#include <DirectXMath.h>

#ifdef UNICODE
#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console")
#else
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#endif

using namespace std;

#define SERVERPORT 4000
#define BUFSIZE 512

void err_quit(const char* msg);

void err_display(const char* msg);

int recvn(SOCKET s, char* buf, int len, int flags);

// int netInit(int argc, char* argv[]);

int netInit();

char* sendKey(char* keybuf);

int netclose();

void do_send(char* keybuf);
