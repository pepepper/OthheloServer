#pragma once
#include <memory>
#include <list>
#include <vector>
#include <sstream>
#include <iostream>
#include "game.h"
#include <errno.h>  
#include <atomic>
#ifdef __linux__
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
typedef SOCKET int;
#else
#include <winsock2.h>
#endif


class othheloserver {
public:
	othheloserver();
	~othheloserver();
	void Run();
	int connected;
	std::list<std::shared_ptr<Game>> Games;
	std::list<std::shared_ptr<Game>> AutoGames;
	SOCKET sock, sock0;
	struct sockaddr_in addr;
	struct sockaddr_in client;
	int end;
	int len;
	std::thread thread, endthread;
private:
#ifndef __linux__
	WSADATA wsaData;
#endif
};