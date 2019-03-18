#pragma once
#include <memory>
#include <list>
#include <vector>
#include <sstream>
#include <iostream>
#include "game.h"
#include <errno.h>  
#ifdef Linux_System
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
typedef int SOCKET;
#else
#include <winsock2.h>
#endif

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