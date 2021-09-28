#pragma comment(lib, "ws2_32")

#include <WS2tcpip.h>
#include <stdlib.h>
#include "Player.h"

#define SERVERPORT 4000
#define BUFSIZE 512

using namespace std;

const char* SERVER_ADDR = "127.0.0.1";

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
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET listen_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	// inet_pton(AF_INET, SERVER_ADDR, &serveraddr.sin_addr);
	retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	char buf[BUFSIZE];

	CPlayer* m_pPlayer = new CPlayer[1];

	while (1) {
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accpet()");
			break;
		}

		// 접속한 클라이언트 정보 출력
		cout << endl << "[TCP 서버] 클라이언트 접속 : IP주소 = " << inet_ntoa(clientaddr.sin_addr) <<
			", 포트번호 = " << ntohs(clientaddr.sin_port) << endl;

		

		// 클라이언트와 데이터 통신
		while (1) {
			// 데이터 받기
			retval = recv(client_sock, buf, BUFSIZE, 0);
			if (retval == SOCKET_ERROR) { err_display("recv()"); break; }
			else if (retval == 0) break;

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
		}

		// close socket()
		closesocket(client_sock);
		cout << endl << "[TCP 서버] 클라이언트 종료 : IP주소 = " << inet_ntoa(clientaddr.sin_addr) <<
			", 포트번호 = " << ntohs(clientaddr.sin_port) << endl;
	}

	// close socket()
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}