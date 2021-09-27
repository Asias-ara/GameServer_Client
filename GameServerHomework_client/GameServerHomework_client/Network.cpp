#include "Network.h"

WSADATA wsa;
SOCKET sock;
SOCKADDR_IN serveraddr;
int retval = 0;

void err_quit(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPTSTR)lpMsgBuf, (LPCTSTR)msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

void err_display(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCWSTR)msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
}

int recvn(SOCKET s, char* buf, int len, int flags)
{
	int received;
	char* ptr = buf;
	int left = len;

	while (left > 0) {
		received = recv(s, ptr, left, flags);
		if (received == SOCKET_ERROR) return SOCKET_ERROR;
		else if (received == 0) break;
		left -= received;
		ptr += received;
	}
	return (len - left);
}

int netInit()
{

	const char* SERVERIP;
	char tempIP[16];
	std::cout << "IP주소를 입력하세요 : ";
	std::cin >> tempIP;
	SERVERIP = tempIP;

	std::cout << SERVERIP << endl;
	// 윈속 초기화
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	sock = socket(AF_INET, SOCK_STREAM, 0);

	// socket()
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serveraddr.sin_port = htons(SERVERPORT);
	// inet_pton(AF_INET, SERVER_ADDR, &serveraddr.sin_addr);
	retval = connect(sock, reinterpret_cast<sockaddr *>(&serveraddr), sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	//// 데이터 통신에 사용할 변수
	//char buf[BUFSIZE];
	//int len;

	//// 서버와 데이터 통신
	//while (1) {
	//	// 데이터 입력
	//	cout << endl << "보낼데이터 : ";
	//	if (fgets(buf, BUFSIZE + 1, stdin) == NULL) break;

	//	// '\n'문자 제거
	//	len = strlen(buf);
	//	if (buf[len - 1] == '\n') buf[len - 1] = '\0';
	//	if (strlen(buf) == 0) break;

	//	// 데이터 보내기
	//	retval = send(sock, buf, strlen(buf), 0);
	//	if (retval == SOCKET_ERROR) { err_display("send()"); break; }
	//	cout << "[TCP 클라이언트] " << retval << "바이트를 보냈습니다" << endl;

	//	// 데이터 받기
	//	retval = recv(sock, buf, retval, 0); cout << "[TCP 클라이언트] " << retval << "바이트를 보냈습니다" << endl;
	//	if (retval == SOCKET_ERROR) { err_display("rev()"); break; }
	//	else if (retval == 0) break;

	//	// 받은 데이터 출력
	//	buf[retval] = '\0';
	//	cout << "[TCP 클라이언트] " << retval << "바이트를 받았습니다" << endl;
	//	cout << "[받은데이터] " << buf << endl;

	//}

	//// close socket()
	//closesocket(sock);

	//// 윈속종료
	//WSACleanup();
	return 0;
}

DWORD sendKey(char* keybuf)
{
	char buf[BUFSIZE];
	int len;
	DWORD send_byte;
	WSABUF mybuf;

	strcpy_s(buf, BUFSIZE, keybuf);

	// '\n'문자 제거
	len = strlen(buf);
	if (buf[len - 1] == '\n') buf[len - 1] = '\0';
	mybuf.buf = buf;
	mybuf.len = len;

	// 데이터 보내기
	retval = WSASend(sock, &mybuf, 1, &send_byte, 0, 0, 0);
	if (retval == SOCKET_ERROR) { err_display("send()");}

	// 데이터 받기
	char recv_buf[BUFSIZE];
	WSABUF mybuf_r;
	mybuf_r.buf = recv_buf;
	mybuf_r.len = BUFSIZE;
	DWORD recv_byte;
	DWORD recv_flag = 0;
	retval = WSARecv(sock, &mybuf_r, 1, &recv_byte, &recv_flag, 0, 0); cout << "[TCP 클라이언트] " << retval << "바이트를 보냈습니다" << endl;
	if (retval == SOCKET_ERROR) { err_display("rev()"); }

	// 받은 데이터 처리
	if (strcmp(buf, "up") == 0)
		return 0x01;
	
	return NULL;
}

int netclose()
{
	// close socket()
	closesocket(sock);

	// 윈속종료
	WSACleanup();
	return 0;
}