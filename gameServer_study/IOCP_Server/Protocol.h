#pragma once

const int MAX_NAME_SIZE = 20;

// 패킷 타입 정의
const char CS_PACKET_LOGIN = 1;
const char CS_PACKET_MOVE = 2;

const char SC_PACKET_LOGIN_OK = 1;
const char SC_PACKET_MOVE = 2;
const char SC_PACKET_PUT_OBJECT = 3;
const char SC_PACKET_REMOVE_OBJECT = 1;

#pragma pack(push, 1)
struct cs_packet_login {
	unsigned char size;
	char type;
	char name[MAX_NAME_SIZE];
};

struct cs_packet_move {
	unsigned char size;
	char type;
	char direction;			// 0 : up, 1 : down, 2 : left, 3: right
};


struct sc_packet_login_ok {
	unsigned char size;
	char type;
	int id;
	short x, y;
};

struct sc_packet_move {
	char size;
	char type;
	int id;
	short x, y;
};

struct sc_packet_pu_object {
	char size;
	char type;
	int id;
	char name[MAX_NAME_SIZE];
	char o_type;
	short x, y;
};
#pragma pack(pop)
