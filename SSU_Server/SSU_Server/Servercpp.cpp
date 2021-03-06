#pragma comment(lib, "WS2_32.LIB")
#pragma comment(lib, "MSWsocke.LIB")

#include <iostream>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <array>
#include "protocol.h"

using namespace std;

// Send/Recv/Accept 구별
enum COMP_OP{OP_RECV, OP_SEND, OP_ACCEPT};

class OVER_EX
{
public:
	WSAOVERLAPPED	_wsa_over;
	WSABUF			_wsa_buf;
	unsigned char	_net_buf[BUFSIZE];
	COMP_OP			_comp_op;

public:
	OVER_EX(COMP_OP comp_op, char num_bytes, void* mess) : _comp_op(comp_op)
	{
		ZeroMemory(&_wsa_over, sizeof(_wsa_over));
		_wsa_buf.buf = reinterpret_cast<char*>(_net_buf);
		_wsa_buf.len = num_bytes;
		memcpy(_net_buf, mess, num_bytes);
	}

	OVER_EX(COMP_OP comp_op) : _comp_op(comp_op) {}

	OVER_EX()		// array에 넣기 위해서는 기본생성자가 있어야한다
	{
		_comp_op = OP_RECV;
	}

	~OVER_EX()
	{

	}
};

class CLIENT
{
public:
	OVER_EX _recv_over;
	SOCKET	_sock;
	int _prev_size;
	int _id;

	bool _use;

	char name[MAX_ID_LEN];
	int x, y;
	int hp, mp;
	int physical_attack, magical_attack;
	int physical_defense, magical_defense;
	int element;
	short level;
	int exp;
public:
	CLIENT() 
	{
		_use = false;
		_prev_size = 0;

		// 일단 임시로 적어놈
		x = 0;
		y = 0;
		level = 50;
		hp = 54000;
		mp = 27500;
		physical_attack = 1250;
		magical_attack = 500;
		physical_defense = 1100;
		magical_defense  = 925;
	}
	~CLIENT() 
	{
		closesocket(_sock);
	}

	void do_recv()
	{
		DWORD recv_flag = 0;
		ZeroMemory(&_recv_over._wsa_over, sizeof(_recv_over._wsa_over));
		_recv_over._wsa_buf.buf = reinterpret_cast<char*>(_recv_over._net_buf + _prev_size);
		_recv_over._wsa_buf.len = sizeof(_recv_over._net_buf) - _prev_size;
		int ret = WSARecv(_sock, &_recv_over._wsa_buf, 1, 0, &recv_flag, &_recv_over._wsa_over, NULL);
		if (ret == SOCKET_ERROR) {
			int err_num = WSAGetLastError();
			// display_error(err_num);
		}
	}

	void do_send(int num_bytes, void* mess)
	{
		OVER_EX* ex_over = new OVER_EX(OP_SEND, num_bytes, mess);
		WSASend(_sock, &ex_over->_wsa_buf, 1, 0, 0, &ex_over->_wsa_over, NULL);
	}
};

class MONSTER 
{

};

array<CLIENT, MAX_USER> clients;

array<MONSTER, 2> monsters;

int get_new_id()
{
	static int g_id = 0;
	for (int i = 0; i < MAX_USER; ++i) {
		if (clients[i]._use == false) {
			clients[i]._use = true;
			return i;
		}
	}

	return -1;
}

void Disconnect(int c_id)
{
	clients[c_id]._use = false;
	for (auto& cl : clients) {
		if (false == cl._use) continue;
		// 연결이 끊겼다는 패킷을 보내주는 함수를 만들자
	}
	closesocket(clients[c_id]._sock);
}

void process_packet(int c_id, unsigned char* p)
{
	CLIENT& cl = clients[c_id];
	int size = p[0];
	int type = p[1];

	switch (type) {
	case CS_PACKET_LOGIN: {
		cs_packet_login* packet = reinterpret_cast<cs_packet_login*>(p);
		// strcpy_s(cl.name, packet->name);
		// send_login_ok_packet(client_id);

		// 기존 클라에게 새로 접속한 클라의 정보를 보내줌
		for (auto& other : clients) {
			if (other._id == c_id) continue;
			if (other._use == false) continue;
			sc_packet_put_object packet;
			
			packet.size = sizeof(packet);
			packet.type = SC_PACKET_PUT_OBJECT;
			strcpy_s(packet.name, cl.name);
			packet.id = c_id;

			packet.x = cl.x;
			packet.y = cl.y;
			packet.hp = cl.hp;
			packet.mp = cl.mp;
			packet.physical_attack = cl.physical_attack;
			packet.magical_attack = cl.magical_attack;
			packet.physical_defense = cl.physical_defense;
			packet.magical_defense = cl.magical_defense;
			packet.element = cl.element;
			packet.level = cl.level;
			packet.exp = cl.exp;
			other.do_send(sizeof(packet), &packet);
		}

		// 새로 접속한 클라에게 현재 월드 상태를 보내줌
		// 1. 접속해 있는 클라
		for (auto& other : clients) {
			if (other._id == c_id) continue;
			if (other._use == false) continue;
			sc_packet_put_object packet;

			packet.size = sizeof(packet);
			packet.type = SC_PACKET_PUT_OBJECT;
			strcpy_s(packet.name, cl.name);
			packet.id = c_id;

			packet.x = cl.x;
			packet.y = cl.y;
			packet.hp = cl.hp;
			packet.mp = cl.mp;
			packet.physical_attack = cl.physical_attack;
			packet.magical_attack = cl.magical_attack;
			packet.physical_defense = cl.physical_defense;
			packet.magical_defense = cl.magical_defense;
			packet.element = cl.element;
			packet.level = cl.level;
			packet.exp = cl.exp;

			cl.do_send(sizeof(packet), &packet);
		}
		// 2. 몬스터의 정보
		// <---------- 어떻게 보내줄까......------------->

	}break;
	case CS_PACKET_MOVE: {

		// 이동이 타당한지 판단한다

		// 이동이 타당하고 이동을 시켰다면 이동한 정보를 모든 클라에게 보내주자

	}break;
	case CS_PACKET_ATTACK: {
		
		// 현재 전투에 들어갈만한 몬스터가 있는지 확인

		// 만약 있다면 몬스터의 정보를 보내주자

		// 다른 클라들에게는 전투에 돌입했다는 정보를 보내주자

	}break;
	}
}

int main()
{
	wcout.imbue(locale("korean"));

	// 윈속 초기화
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	// listen 소켓 생성
	SOCKET server_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_port = htons(SERVERPORT);
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	// bind()
	bind(server_sock, reinterpret_cast<SOCKADDR*>(&server_addr), sizeof(server_addr));

	// listen()
	listen(server_sock, SOMAXCONN);

	// iocp 핸들 객체 생성
	HANDLE h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	// 핸들에 소켓 연결
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(server_sock), h_iocp, 0, 0);

	// 클라 소켓 생성 -> AcceptEx
	SOCKET client_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	char accept_buf[sizeof(SOCKADDR_IN) * 2 + 32 + 100];
	OVER_EX accept_ex;

	ZeroMemory(&accept_ex, sizeof(accept_ex));
	accept_ex._comp_op = OP_ACCEPT;
	
	AcceptEx(server_sock, client_sock, accept_buf, 0,
		sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, NULL, &accept_ex._wsa_over);

	while (1) {
		DWORD num_byte;		// 전송받거나 전송될 데이터의 양
		LONG64 iocp_key;	// 미리 정해 놓은 ID
		WSAOVERLAPPED* p_over;
		BOOL ret = GetQueuedCompletionStatus(h_iocp, &num_byte, (PULONG_PTR)&iocp_key, &p_over, INFINITE);

		int client_id = static_cast<int>(iocp_key);
		OVER_EX* over_ex = reinterpret_cast<OVER_EX*>(p_over);

		// 예외처리 (보통은 연결이 끊어진 것이고 연결이 끊어진것을 처리 해주어야 한다)
		if (ret == false){
			Disconnect(client_id);
			if (over_ex->_comp_op == OP_SEND)
				delete over_ex;			// 잘못 받은것을 삭제해 주자
			continue;
		}

		switch (over_ex->_comp_op) {
		case OP_RECV: {
			CLIENT& cl = clients[client_id];
			int remain_data = num_byte + cl._prev_size;
			unsigned char* packet_start = over_ex->_net_buf;
			int packet_size = packet_start[0];
			
			while (packet_size <= remain_data) {
				// 패킷 처리
				process_packet(client_id, packet_start);
				remain_data -= packet_size;
				packet_start += packet_size;
				if (remain_data > 0) packet_size = packet_start[0];
				else break;
			}

			// 나머지가 있으면 처리
			if (0 < remain_data) {
				cl._prev_size = remain_data;
				memcpy(&over_ex->_net_buf, packet_start, remain_data);
			}

			cl.do_recv();

		} break;

		case OP_SEND: {

		} break;

		case OP_ACCEPT: {
			int new_id = get_new_id();
			CLIENT& cl = clients[new_id];

			// 클라의 정보 초기화
			cl.x = 0;
			cl.y = 0;
			cl.level = 50;
			cl.hp = 54000;
			cl.mp = 27500;
			cl.physical_attack = 1250;
			cl.magical_attack = 500;
			cl.physical_defense = 1100;
			cl.magical_defense = 925;

			// 통신을 위한 변수들 초기화
			cl._prev_size = 0;
			cl._recv_over._comp_op = OP_RECV;
			cl._recv_over._wsa_buf.buf = reinterpret_cast<char*>(cl._recv_over._net_buf);
			cl._recv_over._wsa_buf.len = sizeof(cl._recv_over._net_buf);
			ZeroMemory(&cl._recv_over._wsa_over, sizeof(cl._recv_over._wsa_over));
			cl._sock = client_sock;

			CreateIoCompletionPort(reinterpret_cast<HANDLE>(cl._sock), h_iocp, new_id, 0);
			cl.do_recv();

			// Accept가 완료가 되었으니 다시 accept를 해주어야 한다
			ZeroMemory(&accept_ex._wsa_over, sizeof(accept_ex._wsa_over));
			client_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
			AcceptEx(server_sock, client_sock, accept_buf, 0, sizeof(SOCKADDR_IN) + 16,
				sizeof(SOCKADDR_IN) + 16, NULL, &accept_ex._wsa_over);
		} break;
		}

	}

	closesocket(server_sock);
	WSACleanup();
	return 0;
}
