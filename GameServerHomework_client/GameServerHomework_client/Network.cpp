#include "Network.h"

void err_quit(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCWSTR)msg, MB_ICONERROR);
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
	cout << "[" << msg << "] " << (char*)lpMsgBuf;
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
	int retval;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	//// ������ ��ſ� ����� ����
	//char buf[BUFSIZE];
	//int len;

	//// ������ ������ ���
	//while (1) {
	//	// ������ �Է�
	//	cout << endl << "���������� : ";
	//	if (fgets(buf, BUFSIZE + 1, stdin) == NULL) break;

	//	// '\n'���� ����
	//	len = strlen(buf);
	//	if (buf[len - 1] == '\n') buf[len - 1] = '\0';
	//	if (strlen(buf) == 0) break;

	//	// ������ ������
	//	retval = send(sock, buf, strlen(buf), 0);
	//	if (retval == SOCKET_ERROR) { err_display("send()"); break; }
	//	cout << "[TCP Ŭ���̾�Ʈ] " << retval << "����Ʈ�� ���½��ϴ�" << endl;

	//	// ������ �ޱ�
	//	retval = recv(sock, buf, retval, 0); cout << "[TCP Ŭ���̾�Ʈ] " << retval << "����Ʈ�� ���½��ϴ�" << endl;
	//	if (retval == SOCKET_ERROR) { err_display("rev()"); break; }
	//	else if (retval == 0) break;

	//	// ���� ������ ���
	//	buf[retval] = '\0';
	//	cout << "[TCP Ŭ���̾�Ʈ] " << retval << "����Ʈ�� �޾ҽ��ϴ�" << endl;
	//	cout << "[����������] " << buf << endl;

	//}

	//// close socket()
	//closesocket(sock);

	//// ��������
	//WSACleanup();
	//return 0;
}