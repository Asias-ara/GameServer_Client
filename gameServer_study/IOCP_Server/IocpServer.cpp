#include <iostream>
#include <WS2tcpip.h>		// C언어 표준이 아니라서 라이브러리 추가해야됨
#include <unordered_map>
#include <MSWSock.h>
#include "Protocol.h"

#pragma comment(lib, "ms_WSock.LIB")
#pragma comment(lib, "ws2_32.LIB")

using namespace std;

const int BUFSIZE = 256;
const short SERVER_PORT = 4000;

bool g_shutdown = false;

void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED recv_over, DWORD recv_flag);
void CALLBACK send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED send_over, DWORD send_flag);

class EX_OVER {
public:
	WSAOVERLAPPED	_wsa_over;
	int				_s_id;
	WSABUF			_wsa_buf;
	char			_netbuf[BUFSIZE];
public:
	EX_OVER()
	{

	}

	EX_OVER(char s_id, char num_bytes, char* mess) : _s_id(s_id)
	{
		ZeroMemory(&_wsa_over, sizeof(_wsa_over));
		_wsa_buf.buf = _netbuf;
		_wsa_buf.len = num_bytes + 2;

		memcpy(_netbuf + 2, mess, num_bytes);
		// 사이즈 검사를 원래 넣어야 한다.
		_netbuf[0] = num_bytes + 2;
		// 100명 이상 동접을 하면 id가 겹친다
		_netbuf[1] = s_id;
	}

	~EX_OVER()
	{

	}
};

class CLIENT {
private:
	int id;
	short x, y;
	char name[MAX_NAME_SIZE];

	SOCKET c_sock;
	EX_OVER recv_over;
	int prev_size;

public:
	CLIENT()
	{
		x = 0;
		y = 0;

		init_ex_over(recv_over);
		prev_size = 0;
	}
	CLIENT(int c_id) : id(c_id)
	{
		CLIENT();
	}
	~CLIENT()
	{

	}
};

class SESSION {
private:
	int		_id;
	SOCKET	_socket;
	WSABUF	_recv_wsabuf;
	WSABUF	_send_wsabuf;
	char	_recv_buf[BUFSIZE];
	WSAOVERLAPPED _recv_over;
public:

	SESSION()
	{
		cout << "Unexpected Constructor Call Error \n";
		exit(-1);
	}

	SESSION(int id, SOCKET s) : _id(id), _socket(s)
	{
		_recv_wsabuf.buf = _recv_buf;
		_recv_wsabuf.len = BUFSIZE;
		_send_wsabuf.buf = _recv_buf;
		_send_wsabuf.len = 0;
	}
	~SESSION()
	{
		closesocket(_socket);
	}

	void do_recv()
	{
		// recv()
		DWORD recv_flag = 0;
		ZeroMemory(&_recv_over, sizeof(_recv_over));
		_recv_over.hEvent = reinterpret_cast<HANDLE>(_id);
		WSARecv(_socket, &_recv_wsabuf, 1, 0, &recv_flag, &_recv_over, recv_callback);
	}

	void do_send(int sender_id, int num_bytes, char* mess)
	{
		// send()
		// EXP_OVER* ex_over = new EXP_OVER(sender_id, num_bytes, mess);

		// WSASend(_socket, &ex_over->_wsa_buf, 1, 0, 0, &ex_over->_wsa_over, send_callback);
	}

	char* recv_buf()
	{
		return _recv_buf;
	}
};

array<CLIENT, 10> clients;	

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

/*
void CALLBACK send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED send_over, DWORD send_flag)
{
	EXP_OVER* ex_over = reinterpret_cast<EXP_OVER*>(send_over);
	delete ex_over;
}

void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED recv_over, DWORD recv_flag)
{
	int s_id = reinterpret_cast<int>(recv_over->hEvent);

	cout << "Client[" << s_id << "] sent : [" << num_bytes << "] bytes" << clients[s_id].recv_buf() << endl;

	for (auto& cl : clients) {
		cl.second.do_send(s_id, num_bytes, clients[s_id].recv_buf());
	}

	clients[s_id].do_recv();
}*/


int main()
{
	wcout.imbue(locale("korean"));

	// 윈속 초기화
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	// socket()
	SOCKET s_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);

	// bind()
	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	bind(s_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));

	// listen()
	listen(s_socket, SOMAXCONN);


	// iocp
	HANDLE h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(s_socket), h_iocp, 0, 0);

	SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	char accept_buf[sizeof(SOCKADDR_IN) * 2 + 32 + 100];
	EX_OVER accept_ex;
	AcceptEx(s_socket, c_socket,  accept_buf, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, NULL, &accept_ex._wsa_over);

	for (;;) {
		// 이부분 변수 선언 안한 부분은 다음시간에 하기로 함
		GetQueuedCompletionStatus(h_iocp, num_byte, c_key, wsa_over, INFINITE);
	}


	//// accept()
	//INT addr_size = sizeof(server_addr);
	//for (int i = 1; ; ++i) {
	//	if (true == g_shutdown) break;

	//	SOCKET c_socket = WSAAccept(s_socket, reinterpret_cast<sockaddr*>(&server_addr), &addr_size, 0, 0);
	//	int tcp_option = 1;
	//	setsockopt(c_socket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&tcp_option), sizeof(tcp_option));
	//	clients.try_emplace(i, i, c_socket);		//copy construct를 막기위해 emplace를 함
	//	clients[i].do_recv();
	//}

	// clients.clear();
	closesocket(s_socket);

	WSACleanup();
}

