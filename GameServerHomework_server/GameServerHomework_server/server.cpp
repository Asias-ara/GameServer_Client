#pragma comment(lib, "ws2_32")
#include <WS2tcpip.h>
#include <stdlib.h>
#include <locale.h>
#include <unordered_map>
#include "Player.h"
using namespace std;

#define SERVERPORT 4000
#define BUFSIZE 512

// 함수 선언
void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED recv_over, DWORD recv_flag);
void CALLBACK send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED send_over, DWORD send_flag);
void err_quit(const char* msg);
void err_display(const char* msg);
void move_player();

// 클래스 선언

// 패킷
class EXP_OVER {
public:
	WSAOVERLAPPED	_wsa_over;
	int				_type;
	int				_s_id;
	WSABUF			_wsa_buf;
	char			_msg[BUFSIZE];		// num_bytes를 하면 new할당하며 오버헤드 일어나므로 통크게 확장
public:
	EXP_OVER()
	{

	}

	EXP_OVER(char type, char s_id, char num_bytes, char* mess) : _s_id(s_id)
	{
		ZeroMemory(&_wsa_over, sizeof(_wsa_over));
		_wsa_buf.buf = _msg;
		_wsa_buf.len = num_bytes + 3;

		memcpy(_msg + 3, mess, num_bytes);
		cout << mess << endl;
		// 어떤 타입인지 넣어준다( 1 : 로그인, 2 : 이동, 3 : 로그아웃)
		_msg[0] = type;
		// 사이즈 검사를 원래 넣어야 한다.
		_msg[1] = num_bytes + 3;
		// 100명 이상 동접을 하면 id가 겹친다
		_msg[2] = s_id;
	}

	~EXP_OVER()
	{

	}
};

// 세션
class SESSION {
private:
	int		_id;
	SOCKET	_socket;
	WSABUF	_recv_wsabuf;
	WSABUF	_send_wsabuf;
	char	_buf[BUFSIZE];
	WSAOVERLAPPED _recv_over;
public:

	SESSION()
	{
		cout << "Unexpected Constructor Call Error \n";
		exit(-1);
	}

	SESSION(int id, SOCKET s) : _id(id), _socket(s)
	{
		_recv_wsabuf.buf = _buf;
		_recv_wsabuf.len = BUFSIZE;
		_send_wsabuf.buf = _buf;
		_send_wsabuf.len = 0;
	}
	~SESSION()
	{
		closesocket(_socket);
	}

	void do_recv()
	{
		cout << "recv여긴 되냐1" << endl;
		// recv()
		int type = 0;
		DWORD recv_flag = 0;
		ZeroMemory(&_recv_over, sizeof(_recv_over));

		
		// EXP_OVER* ex_over = new EXP_OVER(0, _id, BUFSIZE, NULL);

		// int ret = WSARecv(_socket, &_recv_wsabuf, 1, 0, &recv_flag, &ex_over->_wsa_over, recv_callback);
		_recv_over.hEvent = reinterpret_cast<HANDLE>(_id);
		int ret = WSARecv(_socket, &_recv_wsabuf, 1, 0, &recv_flag, &_recv_over, recv_callback);
		if (ret == SOCKET_ERROR) {
			int err_num = WSAGetLastError();
			if (WSA_IO_PENDING != err_num) {
				if (WSAECONNRESET == err_num) {
					closesocket(_socket);
				}
				else {
					cout << " EROOR : RECV " << endl;
					err_display("recv()");
				}
			}
		}
	}

	void do_send(int type, int sender_id, int num_bytes, char* mess)
	{
		cout << "send되냐1" << mess << endl;
		// send()
		EXP_OVER* ex_over = new EXP_OVER(type, sender_id, num_bytes, mess);
		
		int ret = WSASend(_socket, &ex_over->_wsa_buf, 1, 0, 0, &ex_over->_wsa_over, send_callback);
		if (ret == SOCKET_ERROR) {
			int err_num = WSAGetLastError();
			if (WSA_IO_PENDING != err_num) {
				cout << " EROOR : SEND " << endl;
				err_display("send()");
			}
	
		}
	}

	char* recv_buf()
	{
		return _buf;
	}
};

unordered_map<int, SESSION> clients;

void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED recv_over, DWORD recv_flag)
{
	cout << "recv_callback여긴 되냐2" << endl;
	int s_id = reinterpret_cast<int>(recv_over->hEvent);

	cout << "Client[" << s_id << "] sent : [" << num_bytes << "] bytes" << clients[s_id].recv_buf() << endl;

	for (auto& cl : clients) {
		//cl.second.do_send(s_id, num_bytes, clients[s_id].recv_buf());
	}

	clients[s_id].do_recv();
	/*
	EXP_OVER* ex_over = reinterpret_cast<EXP_OVER*>(recv_over);
	char* recv_buf = ex_over->_msg;

	char* p = recv_buf;

	while (p < recv_buf + num_bytes) {
		int type = *p;
		char packet_size = *(p + 1);
		int c_id = *(p + 2);
		p = p + packet_size;

		switch (type) {
		case 1:			// 로그인
			cout << "로그인 되었습니다" << endl;
			break;
		case 2:			// 위치 변경
			break;
		case 3:			// 로그아웃
			break;
		}

		cout << "Client[" << c_id << "] sent : [" << num_bytes << "] bytes" << clients[c_id].recv_buf() << endl;
		for (auto& cl : clients) {
			cl.second.do_send(type, c_id, num_bytes, clients[c_id].recv_buf());
		}

		clients[c_id].do_recv();
	}

	delete ex_over;
	*/
}

void CALLBACK send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED send_over, DWORD send_flag)
{
	cout << "send_callbakc되냐2" << endl;
	EXP_OVER* ex_over = reinterpret_cast<EXP_OVER*>(send_over);
	delete ex_over;
}

#pragma pack (push, 1)
struct move_packet {
	short size;
	char type;
	float x, y, z;
	float dx, dy, dz;
};
#pragma pack (pop)

void err_quit(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	std::wcout.imbue(std::locale("korean"));
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCWSTR)msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
};

void err_display(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	std::wcout.imbue(std::locale("korean"));
	cout << "[" << msg << "] " << (char*)lpMsgBuf;
	LocalFree(lpMsgBuf);
}

int main(int argc, char* argv[])
{
	wcout.imbue(locale("korean"));
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET listen_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");
	
	int i = 1;
	while (1) {
		// accept()
		SOCKADDR_IN c_addr;
		INT addrlen = sizeof(c_addr);
		SOCKET c_socket = WSAAccept(listen_sock, reinterpret_cast<sockaddr*>(&c_addr), &addrlen, 0, 0);
		retval = WSAGetLastError();
		if (retval == INVALID_SOCKET) {
			err_display("accpet()");
			break;
		}

		// no_delay
		int tcp_option = 1;
		setsockopt(c_socket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&tcp_option), sizeof(tcp_option));

		// clients에 입력
		cout << "[TCP/" << inet_ntoa(c_addr.sin_addr) << ":" << ntohs(c_addr.sin_port) << "]" << endl;
		clients.try_emplace(i, i, c_socket);
		

		char mess[6] = "login";
		cout << strlen(mess) << endl;
		for (auto& cl : clients) {
			cl.second.do_send(1, i, 6, mess);
		}

		clients[i].do_recv();
	}

	// close socket()
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}

void move_player()
{/*
	// 받은 데이터 출력
	buf[retval] = '\0';
	cout << "[TCP/" << inet_ntoa(clientaddr.sin_addr) << ":" << ntohs(clientaddr.sin_port) << "]" << buf << endl;

	// 받은 데이터를 처리하기
	DWORD dwDirection = 0;
	if (strcmp(buf, "up") == 0) dwDirection |= 0x01;
	if (strcmp(buf, "down") == 0) dwDirection |= 0x02;
	if (strcmp(buf, "left") == 0) dwDirection |= 0x04;
	if (strcmp(buf, "right") == 0) dwDirection |= 0x08;

	if (dwDirection) {
		m_pPlayer->Move(dwDirection, 50.0f * 0.01, true);
		m_pPlayer->printPosition();
	}

	DWORD send_byte;
	XMFLOAT3* sendFloat;
	char p[BUFSIZE];

	*sendFloat = m_pPlayer->sendPosition();
	*((short*)(&buf[0])) = 14;
	*((float*)(&buf[2])) = (*sendFloat).x;		// x좌표
	*((float*)(&buf[6])) = (*sendFloat).y;		// y좌표
	*((float*)(&buf[10])) = (*sendFloat).z;		// z좌표

	// 데이터 보내기
	retval = send(client_sock, buf, *((short*)&buf[0]), 0);
	if (retval == SOCKET_ERROR) { err_display("send()"); break; }
	*/
}