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
#include <netdb.h>
#include <unistd.h>
typedef int SOCKET;
#define INVALID_SOCKET -1
#define closesockert(x) close(x)
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
	struct sockaddr_storage addr;
	struct sockaddr_storage client;
	int end;
#ifdef __linux__
	socklen_t len;
#else
	int len;
#endif
	std::thread thread, endthread;
private:
#ifndef __linux__
	WSADATA wsaData;
#endif
	struct addrinfo *res=NULL;
};
