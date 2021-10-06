#include <iostream>
#include <WS2tcpip.h>		// C언어 표준이 아니라서 라이브러리 추가해야됨

#pragma comment(lib, "ws2_32.LIB")

using namespace std;

const char* SERVER_ADDR = "127.0.0.1";
const int BUFSIZE = 256;
const short SERVER_PORT = 4000;

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


int main()
{
	wcout.imbue(locale("korean"));

	// 윈속 초기화
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	// socket()
	SOCKET s_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, 0);

	// connect()
	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_ADDR, &server_addr.sin_addr);
	connect(s_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	for (;;) {
		// 문자 받기
		char buf[BUFSIZE];
		cin.getline(buf, BUFSIZE);
		
		// send()
		DWORD sent_byte;
		WSABUF mybuf;
		mybuf.buf = buf;
		mybuf.len = static_cast<ULONG>(strlen(buf)) + 1;
		int ret = WSASend(s_socket, &mybuf, 1, &sent_byte, 0, 0, 0);
		if (ret == SOCKET_ERROR) { err_display("send()"); break; }


		// recv()
		char recv_buf[BUFSIZE];
		WSABUF mybuf_recv;
		mybuf_recv.buf = recv_buf;
		mybuf_recv.len = BUFSIZE;
		DWORD recv_byte;
		DWORD recv_flag = 0;
		WSARecv(s_socket, &mybuf_recv, 1, &recv_byte, &recv_flag, 0, 0);
		cout << "Server sent : [" << recv_byte << "] bytes" << recv_buf << endl;
	}
	WSACleanup();
}
