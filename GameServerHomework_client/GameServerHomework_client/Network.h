#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32")

#include <WS2tcpip.h>
#include <iostream>
#include <stdlib.h>
#include <DirectXMath.h>
#include <unordered_map>

#include "stdafx.h"

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


// int netInit(int argc, char* argv[]);

int netInit();

char* sendKey(char* keybuf);

int netclose();

void do_send(char* keybuf);

void do_recv();

// float my_position2 = -1.0f;
XMFLOAT3 return_myPosition();
XMFLOAT3 return_myCamera();