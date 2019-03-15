#pragma once
#include <memory>
#include <list>
#include <vector>
#include <sstream>
#include "game.h"

#ifdef Linux_System
typedef int SOCKET;
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

class othheloserver{
	public:
	othheloserver();
	~othheloserver();
	void Run();
	int connected;
	std::list<std::shared_ptr<Game>> Games;
	SOCKET sock0;
	struct sockaddr_in addr;
	struct sockaddr_in client;
	int end;
	unsigned int len;
	SOCKET sock;
	std::thread thread;
};

#else
#include <winsock2.h>

class othheloserver{
	public:
	othheloserver();
	~othheloserver();
	void Run();
	int connected;
	std::list<std::shared_ptr<Game>> Games;
	WSADATA wsaData;
	SOCKET sock0;
	struct sockaddr_in addr;
	struct sockaddr_in client;
	int len, end;
	SOCKET sock;
	std::thread thread, gamedecon;
};

#endif