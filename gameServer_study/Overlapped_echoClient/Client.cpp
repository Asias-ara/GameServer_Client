#include <iostream>
#include <WS2tcpip.h>		// C언어 표준이 아니라서 라이브러리 추가해야됨

#pragma comment(lib, "ws2_32.LIB")

using namespace std;

const char* SERVER_ADDR = "127.0.0.1";
const int BUFSIZE = 256;
const short SERVER_PORT = 4000;

SOCKET g_s_socket;
char g_recv_buf[BUFSIZE];

WSABUF mybuf_recv;
WSABUF mybuf;

bool g_client_shutdown = false;

void CALLBACK send_callback(DWORD err, DWORD num_byte, LPWSAOVERLAPPED send_over, DWORD flag);

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

void do_send()
{
	// 문자 받기
	char buf[BUFSIZE];
	cin.getline(buf, BUFSIZE);

	// send()
	DWORD sent_byte;
	mybuf.buf = buf;
	mybuf.len = static_cast<ULONG>(strlen(buf)) + 1;

	// Overlapped 추가사항
	WSAOVERLAPPED* send_over = new WSAOVERLAPPED;
	ZeroMemory(send_over, sizeof(send_over));



	int ret = WSASend(g_s_socket, &mybuf, 1, &sent_byte, 0, send_over, send_callback);
	if (ret == SOCKET_ERROR) {
		cout << " ERROR(SEND)" << endl;
		err_display("send()");
	}
}

void CALLBACK recv_callback(DWORD err, DWORD num_byte, LPWSAOVERLAPPED recv_over, DWORD flag)
{
	delete recv_over;

	cout << "Server sent : [" << num_byte << "] bytes" << g_recv_buf << endl;

	do_send();
}

void CALLBACK send_callback(DWORD err, DWORD num_byte, LPWSAOVERLAPPED send_over, DWORD flag)
{
	delete send_over;
	// recv()
	mybuf_recv.buf = g_recv_buf;
	mybuf_recv.len = BUFSIZE;
	DWORD recv_flag = 0;

	WSAOVERLAPPED* recv_over = new WSAOVERLAPPED;
	ZeroMemory(recv_over, sizeof(recv_over));

	int ret = WSARecv(g_s_socket, &mybuf_recv, 1, 0, &recv_flag, recv_over, recv_callback);
	if (ret == SOCKET_ERROR) {
		cout << " ERROR(RECV)" << endl;
		int err_num = WSAGetLastError();
		if (WSA_IO_PENDING != err_num) {
			cout << " EROOR : SEND " << endl;
			err_display("send()");

		}
	}
}



int main()
{

	wcout.imbue(locale("korean"));

	// 윈속 초기화
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	// socket()
	// SOCKET s_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, 0);
	g_s_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	
	// connect()
	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_ADDR, &server_addr.sin_addr);
	connect(g_s_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	do_send();
	while (g_client_shutdown == false)
		SleepEx(100, true);
	closesocket(g_s_socket);
	WSACleanup();
}
