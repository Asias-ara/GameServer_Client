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
void delete_session(int c_id);

// 임시 색깔 지정
float temp_colors[10][3] = { {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.9f, 0.9f, 0.9f}, {0.8f, 0.8f, 0.8f},
							{0.7f, 0.7f, 0.7f} , {0.6f, 0.6f, 0.6f} , {0.6f, 0.6f, 0.6f},
							{0.5f, 0.5f, 0.5f} , {0.4f, 0.4f, 0.4f} , {0.3f, 0.3f, 0.3f} };

// 클래스 선언
#pragma pack (push, 1)
typedef struct player_packet {
	short	size;				// 
	int		id;					// id 값
	float	r, g, b, a;			// 색깔값
	float	x, y, z;			// 위치값
	
}PLAYER;
#pragma pack (pop)

class EXP_OVER {
public:
	WSAOVERLAPPED	_wsa_over;
	int				_type;				// 타입 : 1 - 로그인, 2 - 이동, 3 - 로그아웃
	int				_s_id;				// 클라이언트의 id
	WSABUF			_wsa_buf;			// buf
	char			_msg[BUFSIZE];		// num_bytes를 하면 new할당하며 오버헤드 일어나므로 통크게 확장
public:
	EXP_OVER()
	{

	}

	EXP_OVER( char s_id, unsigned char num_bytes, char* mess, char type) : _s_id(s_id)
	{
		ZeroMemory(&_wsa_over, sizeof(_wsa_over));
		_wsa_buf.buf = _msg;
		_wsa_buf.len = num_bytes + 3;

		memcpy(_msg + 3, mess, num_bytes);
		// 어떤 타입인지 넣어준다( 1 : 로그인, 2 : 이동, 3 : 로그아웃)
		_msg[0] = type;
		// 사이즈 검사를 원래 넣어야 한다.
		if (num_bytes > 255)
			_msg[1] = num_bytes + 3;
		else
			_msg[1] = 34 + 3;
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
	int				_id;
	PLAYER			_player;
	SOCKET			_socket;
	WSABUF			_recv_wsabuf;
	WSABUF			_send_wsabuf;
	char			_buf[BUFSIZE];
	WSAOVERLAPPED	_recv_over;
public:
	bool			_real;

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
		
		// 플레이어 정보 초기화
		_player.size = sizeof(_player);
		_player.id = _id;
		if (id < 10) {
			_player.r = temp_colors[id][0];	_player.g = temp_colors[id][1];	_player.b = temp_colors[id][2];	_player.a = 1;
		}
		_player.x = -1;	_player.y = 5;	_player.z = -1;

		_real = true;
	}
	~SESSION()
	{
		closesocket(_socket);
	}

	void do_recv()
	{
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
					_real = false;
					delete_session(_id);
				}
				else if (err_num == 10038) { _real = false; }
				else {
					cout << " EROOR : RECV " << err_num << endl;
					err_display("recv()");
				}
			}
		}
	}

	void do_send(int type, int sender_id, DWORD num_bytes, char* mess)
	{
		// send()
		int temp_num_bytes = num_bytes;
		EXP_OVER* ex_over = new EXP_OVER( sender_id, temp_num_bytes, mess, type);
		int ret = WSASend(_socket, &ex_over->_wsa_buf, 1, 0, 0, &ex_over->_wsa_over, send_callback);
		
		if (ret == SOCKET_ERROR) {
			int err_num = WSAGetLastError();
			if (WSA_IO_PENDING != err_num) {
				if (WSAECONNRESET == err_num) {
					_real = false;
					delete_session(_id);
				}
				else if (err_num == 10038) { _real = false; }
				else {
					cout << " EROOR : SEND " << endl;
					err_display("send()");
				}
			}
		}
	}

	char* recv_buf()
	{
		return _buf;
	}

	PLAYER* send_player() {
		return &_player;
	}
	void move_player()
	{
		char* mess = _buf + 1;
		// 받은 데이터를 처리하기
		DWORD dwDirection = 0;
		if (strcmp(mess, "up") == 0) dwDirection |= 0x01;
		if (strcmp(mess, "down") == 0) dwDirection |= 0x02;
		if (strcmp(mess, "left") == 0) dwDirection |= 0x04;
		if (strcmp(mess, "right") == 0) dwDirection |= 0x08;

		// 여기서 좌표 수정
		if (dwDirection) {
			if (dwDirection & DIR_FORWARD) _player.z = _player.z + (50.0f * 0.01);
			if (dwDirection & DIR_BACKWARD) _player.z = _player.z - (50.0f * 0.01);;
			//화살표 키 ‘→’를 누르면 로컬 x-축 방향으로 이동한다. ‘←’를 누르면 반대 방향으로 이동한다. 
			if (dwDirection & DIR_RIGHT) _player.x = _player.x + (50.0f * 0.01);
			if (dwDirection & DIR_LEFT)  _player.x = _player.x - (50.0f * 0.01);
		}
	}
};

unordered_map<int, SESSION> clients;

void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED recv_over, DWORD recv_flag)
{
	int s_id = reinterpret_cast<int>(recv_over->hEvent);

	// 시도
	char* p;
	ZeroMemory(&p, sizeof(p));
	p = clients[s_id].recv_buf();
	int packet_size = *p;
	cout << "Client[" << s_id << "] sent : [" << packet_size << "] bytes" << clients[s_id].recv_buf()+1 << endl;
	clients[s_id].move_player();
	
	PLAYER* mess = clients[s_id].send_player();
	int len = mess->size;
	char* p_mess = reinterpret_cast<char*>(mess);
	for (auto& cl : clients) {
		cl.second.do_send(2, s_id, len, p_mess);
	}
	clients[s_id].do_recv();
	//-----------


	/*for (auto& cl : clients) {
		cl.second.do_send(2, s_id, num_bytes, clients[s_id].recv_buf());
	}

	clients[s_id].do_recv();*/
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
	EXP_OVER* ex_over = reinterpret_cast<EXP_OVER*>(send_over);
	delete ex_over;
}

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
		
		// 새로 접속한 클라이언트에게 부여된 식별번호와 로그인 되어 있는 클라이언트들의 정보를 보내주기
		PLAYER* mess = clients[i].send_player();
		int len = mess->size;
		char* p_mess = reinterpret_cast<char*>(mess);
		clients[i].do_send(0, i, len, p_mess);
		
		// 새로 생성된 플레이어 정보 broadcasting
		for (auto& cl : clients) {
			cl.second.do_send(1, i, len, p_mess);
		}
		
		// 새로 접속한 플레이어에게 모든 플레이어 정보 보내주기
		for (auto& cl : clients) {
			if (cl.second._real) {
				mess = cl.second.send_player();
				len = mess->size;
				int id = mess->id;
				p_mess = reinterpret_cast<char*>(mess);
				clients[i].do_send(1, id, len, p_mess);
			}
		}


		clients[i].do_recv();
		i++;
	}

	// close socket()
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}

void delete_session(int c_id)
{
	//delete &clients[c_id];
	//clients.erase(c_id);

	// 여기서 로그아웃한 정보를 보내주자
	clients[c_id].~SESSION();
	char mess[7] = "logout";
	// 새로 생성된 플레이어 정보 계산


	for (auto& cl : clients) {
		cl.second.do_send(3, c_id, 7, mess);
	}
}

