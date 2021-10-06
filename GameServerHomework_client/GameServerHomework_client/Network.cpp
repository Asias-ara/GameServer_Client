#include "Network.h"

WSADATA wsa;
SOCKET sock;
SOCKADDR_IN serveraddr;
int retval = 0;

SOCKET g_s_socket;
char g_recv_buf[BUFSIZE];

WSABUF mybuf_recv;
WSABUF mybuf;

bool g_client_shutdown = false;

void CALLBACK send_callback(DWORD err, DWORD num_byte, LPWSAOVERLAPPED send_over, DWORD flag);
void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED recv_over, DWORD flag);

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

void do_send(char* keybuf)
{
	// 문자 받기
	char buf[BUFSIZE];
	strcpy_s(buf, BUFSIZE, keybuf);
	int len = strlen(buf);
	if (buf[len - 1] == '\n') buf[len - 1] = '\0';
	// cin.getline(buf, BUFSIZE);

	// send()
	DWORD sent_byte;
	mybuf.buf = buf;
	mybuf.len = static_cast<ULONG>(strlen(buf)) + 1;

	// Overlapped 추가사항
	WSAOVERLAPPED* send_over = new WSAOVERLAPPED;
	ZeroMemory(send_over, sizeof(send_over));



	int ret = WSASend(g_s_socket, &mybuf, 1, &sent_byte, 0, send_over, send_callback);
	if (ret == SOCKET_ERROR) {
		int err_num = WSAGetLastError();
		if (WSA_IO_PENDING != err_num) {
			cout << " EROOR : SEND " << endl;
			err_display("send()");
		}
	}
}

void do_recv()
{
	// recv()
	mybuf_recv.buf = g_recv_buf;
	mybuf_recv.len = BUFSIZE;
	DWORD recv_flag = 0;

	WSAOVERLAPPED* recv_over = new WSAOVERLAPPED;
	ZeroMemory(recv_over, sizeof(recv_over));

	int ret = WSARecv(g_s_socket, &mybuf_recv, 1, 0, &recv_flag, recv_over, recv_callback);
	if (ret == SOCKET_ERROR) {
		int err_num = WSAGetLastError();
		if (WSA_IO_PENDING != err_num) {
			cout << " EROOR : RECV " << endl;
			err_display("recv()");

		}
	}
}

void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED recv_over, DWORD flag)
{
	delete recv_over;

	char* p = g_recv_buf;

	while (p < g_recv_buf + num_bytes) {
		int type = *p;
		char packet_size = *(p + 1);
		int c_id = *(p + 2);

		if(type == 1)
			cout << "Server" << c_id << " sent : [" << packet_size - 3 << "] bytes : " << p + 3 << endl;

		p = p + packet_size;
	}

	do_recv();
}

void CALLBACK send_callback(DWORD err, DWORD num_byte, LPWSAOVERLAPPED send_over, DWORD flag)
{
	cout << "요기요기" << endl;
	delete send_over;
	// do_send();
}

int netInit()
{

	const char* SERVERIP;
	char tempIP[16];
	//std::cout << "IP주소를 입력하세요 : ";
	//std::cin >> tempIP;
	//SERVERIP = tempIP;
	SERVERIP = "127.0.0.1";

	std::cout << SERVERIP << endl;

	// 윈속 초기화
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// socket()
	// SOCKET s_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, 0);
	g_s_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	if (g_s_socket == INVALID_SOCKET) err_quit("socket()");

	// connect()
	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVERPORT);
	inet_pton(AF_INET, SERVERIP, &server_addr.sin_addr);
	int ret = connect(g_s_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	int err_num = WSAGetLastError();
	if (ret == SOCKET_ERROR) {
		int err_num = WSAGetLastError();
		if (WSA_IO_PENDING != err_num) {
			cout << " EROOR : Connect " << endl;
			err_quit("connect()");

		}
	}

	// Nodelay설정
	int tcp_option = 1;
	setsockopt(g_s_socket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&tcp_option), sizeof(tcp_option));
	do_recv();

	// do_send();
}

char* sendKey(char* keybuf)
{
	//char buf[BUFSIZE];
	//strcpy_s(buf, BUFSIZE, keybuf);
	//len = strlen(buf);
	//do_send(buf);
	//// '\n'문자 제거
	//if (buf[len - 1] == '\n') buf[len - 1] = '\0';
	//mybuf.buf = buf;
	//mybuf.len = len;


	//
	//return recv_buf;
}

int netclose()
{
	// close socket()
	closesocket(sock);

	// 윈속종료
	WSACleanup();
	return 0;
}