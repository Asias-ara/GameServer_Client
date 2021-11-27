#include <iostream>
#include <WS2tcpip.h>		// C언어 표준이 아니라서 라이브러리 추가해야됨
#include <unordered_map>

#pragma comment(lib, "ws2_32.LIB")

using namespace std;

const int BUFSIZE = 256;
const short SERVER_PORT = 4000;

bool g_shutdown = false;

void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED recv_over, DWORD recv_flag);
void CALLBACK send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED send_over, DWORD send_flag);

class EXP_OVER {
public:
	WSAOVERLAPPED	_wsa_over;
	int				_s_id;
	WSABUF			_wsa_buf;
	char			_send_msg[BUFSIZE];		// num_bytes를 하면 new할당하며 오버헤드 일어나므로 통크게 확장
public:
	EXP_OVER()
	{

	}

	EXP_OVER(char s_id, char num_bytes, char* mess) : _s_id(s_id)
	{
		ZeroMemory(&_wsa_over, sizeof(_wsa_over));
		_wsa_buf.buf = _send_msg;
		_wsa_buf.len = num_bytes+2;

		memcpy(_send_msg + 2, mess, num_bytes);
		// 사이즈 검사를 원래 넣어야 한다.
		_send_msg[0] = num_bytes + 2;
		// 100명 이상 동접을 하면 id가 겹친다
		_send_msg[1] = s_id;
	}

	~EXP_OVER()
	{

	}
};


class SESSION {
private:
	int		_id;
	SOCKET	_socket;
	WSABUF	_recv_wsabuf;
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

	void do_send(int sender_id, int num_bytes, char *mess)
	{
		// send()
		EXP_OVER* ex_over = new EXP_OVER(sender_id, num_bytes, mess);

		WSASend(_socket, &ex_over->_wsa_buf, 1, 0, 0, &ex_over->_wsa_over, send_callback);
	}

	char* recv_buf()
	{
		return _recv_buf;
	}
};

unordered_map<int, SESSION> clients;		// 원래 SESSION에 포인터를 넣어야함(멀티쓰레드 강의에서 할꺼임)

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

void CALLBACK send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED send_over, DWORD send_flag)
{
	EXP_OVER* ex_over = reinterpret_cast<EXP_OVER*>(send_over);
	delete ex_over;
}

void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED recv_over, DWORD recv_flag)
{
	int s_id = reinterpret_cast<int>(recv_over->hEvent);

	cout << "Client[" << s_id << "] sent : [" << num_bytes << "] bytes" << clients[s_id].recv_buf() << endl;

	for (auto &cl : clients){
		cl.second.do_send(s_id, num_bytes, clients[s_id].recv_buf());
	}
	
	clients[s_id].do_recv();
}


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

	// accept()
	INT addr_size = sizeof(server_addr);
	for (int i = 1; ; ++i) {
		if (true == g_shutdown) break;

		SOCKET c_socket = WSAAccept(s_socket, reinterpret_cast<sockaddr*>(&server_addr), &addr_size, 0, 0);
		int tcp_option = 1;
		setsockopt(c_socket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&tcp_option), sizeof(tcp_option));
		clients.try_emplace(i, i, c_socket);		//copy construct를 막기위해 emplace를 함
		clients[i].do_recv();
	}

	clients.clear();
	closesocket(s_socket);

	WSACleanup();
}
