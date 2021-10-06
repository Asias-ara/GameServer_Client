#include <iostream>
#include <WS2tcpip.h>		// C��� ǥ���� �ƴ϶� ���̺귯�� �߰��ؾߵ�
#include <unordered_map>

#pragma comment(lib, "ws2_32.LIB")

using namespace std;

const int BUFSIZE = 256;
const short SERVER_PORT = 4000;

bool g_shutdown = false;

void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED recv_over, DWORD recv_flag);
void CALLBACK send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED send_over, DWORD send_flag);



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

	void do_send(int num_bytes)
	{
		// send()
		// ������ ������ ������ send�� recv�� ������ �����̵ǰ�
		// send�� �Ϸ�Ǿ��� recv�� �ǰ� recv�� �Ϸ�Ǿ��� send�� �Ǳ� ������
		// �׷����� len������ ���� �и��ϴ°� ����.
		ZeroMemory(&_recv_over, sizeof(_recv_over));	
		_recv_over.hEvent = reinterpret_cast<HANDLE>(_id);
		_send_wsabuf.len = num_bytes;
		WSASend(_socket, &_send_wsabuf, 1, 0, 0, &_recv_over, send_callback);
	}

	const char* recv_buf()
	{
		return _recv_buf;
	}
};

unordered_map<int, SESSION> clients;		// ���� SESSION�� �����͸� �־����(��Ƽ������ ���ǿ��� �Ҳ���)

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
	int s_id = reinterpret_cast<int>(send_over->hEvent);
	clients[s_id].do_recv();
}

void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED recv_over, DWORD recv_flag)
{
	int s_id = reinterpret_cast<int>(recv_over->hEvent);

	cout << "Client sent : [" << num_bytes << "] bytes" << clients[s_id].recv_buf() << endl;


	clients[s_id].do_send(num_bytes);
}


int main()
{
	wcout.imbue(locale("korean"));

	// ���� �ʱ�ȭ
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
	for(int i = 1; ; ++i) {
		if (true == g_shutdown) break;

		SOCKET c_socket = WSAAccept(s_socket, reinterpret_cast<sockaddr*>(&server_addr), &addr_size, 0, 0);
		clients.try_emplace(i, i, c_socket);		//copy construct�� �������� emplace�� ��
		clients[i].do_recv();
	}
	
	clients.clear();
	closesocket(s_socket);

	WSACleanup();
}
