#include <iostream>
#include <WS2tcpip.h>		// C��� ǥ���� �ƴ϶� ���̺귯�� �߰��ؾߵ�

#pragma comment(lib, "ws2_32.LIB")

using namespace std;

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

	// ���� �ʱ�ȭ
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	// socket()
	SOCKET s_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, 0);

	// bind()
	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	bind(s_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));

	// listen()
	listen(s_socket, SOMAXCONN);

	for (;;) {
		// accept()
		INT addr_size = sizeof(server_addr);
		SOCKET c_socket = WSAAccept(s_socket, reinterpret_cast<sockaddr*>(&server_addr), &addr_size, 0, 0);

		for (;;) {
			// recv()
			char recv_buf[BUFSIZE];
			WSABUF mybuf;
			mybuf.buf = recv_buf;
			mybuf.len = BUFSIZE;
			DWORD recv_byte;
			DWORD recv_flag = 0;
			WSARecv(c_socket, &mybuf, 1, &recv_byte, &recv_flag, 0, 0);
			cout << "Client sent : [" << recv_byte << "] bytes" << recv_buf << endl;

			// ���� �ޱ�
			// send()
			DWORD sent_byte;
			mybuf.len = recv_byte;
			WSASend(c_socket, &mybuf, 1, &sent_byte, 0, 0, 0);

		}
		closesocket(c_socket);
	}
	WSACleanup();
}
