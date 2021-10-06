#include <iostream>
#include <WS2tcpip.h>		// C언어 표준이 아니라서 라이브러리 추가해야됨

#pragma comment(lib, "ws2_32.LIB")

using namespace std;

const int BUFSIZE = 256;
const short SERVER_PORT = 4000;

SOCKET g_s_socket;
SOCKET g_c_socket;
bool g_shutdown = false;
char g_recv_buf[BUFSIZE];

WSABUF mybuf;

void do_recv();

void err_display(const char* msg)
{
	LPVOID lpMsgBuf;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL);

	cout << "[" << msg << "] " << (char*)lpMsgBuf << endl;
	LocalFree(lpMsgBuf);
}

void err_quit(const char* msg)
{
	LPVOID lpMsgBuf;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL);

	MessageBox(NULL, (LPTSTR)lpMsgBuf, (LPTSTR)msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

void CALLBACK send_callback(DWORD err, DWORD num_byte, LPWSAOVERLAPPED send_over, DWORD send_flag)
{
	delete send_over;

	do_recv();
}

void CALLBACK recv_callback(DWORD err, DWORD num_byte, LPWSAOVERLAPPED recv_over, DWORD recv_flag)
{
	delete recv_over;
	cout << "Client sent : [" << num_byte << "] bytes" << g_recv_buf << endl;

	// 문자 받기
	// send()
	DWORD sent_byte;
	mybuf.buf = g_recv_buf;
	mybuf.len = num_byte;

	WSAOVERLAPPED* send_over = new WSAOVERLAPPED;
	ZeroMemory(send_over, sizeof(*send_over));

	WSASend(g_c_socket, &mybuf, 1, &sent_byte, 0, send_over, send_callback);
}


void do_recv()
{
	// recv()
	mybuf.buf = g_recv_buf;
	mybuf.len = BUFSIZE;
	DWORD recv_flag = 0;

	WSAOVERLAPPED* recv_over = new WSAOVERLAPPED;
	ZeroMemory(recv_over, sizeof(*recv_over));

	WSARecv(g_c_socket, &mybuf, 1, 0, &recv_flag, recv_over, recv_callback);
}


int main()
{
	wcout.imbue(locale("korean"));

	// 윈속 초기화
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	// socket()
	g_s_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);

	// bind()
	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	bind(g_s_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));

	// listen()
	listen(g_s_socket, SOMAXCONN);

	// accept()
	INT addr_size = sizeof(server_addr);
	g_c_socket = WSAAccept(g_s_socket, reinterpret_cast<sockaddr*>(&server_addr), &addr_size, 0, 0);
	
	do_recv();
	while (g_shutdown == false)
		SleepEx(100, true);

	closesocket(g_c_socket);
	closesocket(g_s_socket);
	WSACleanup();
}
