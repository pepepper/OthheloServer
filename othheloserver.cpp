#include "othheloserver.h"

std::vector<std::string> spritstring(std::string str) {
	std::vector<std::string> ret;
	std::string temp;
	std::stringstream ss{ str };
	while (std::getline(ss, temp, ' ')) {
		ret.push_back(temp);
	}
	return ret;
}
othheloserver::othheloserver() :connected(0), end(0),res(NULL) {
	struct addrinfo hints={0};
	struct addrinfo *ai;
	hints.ai_family=AF_INET6;
	hints.ai_socktype=SOCK_STREAM;
	hints.ai_flags=AI_PASSIVE;
#ifndef __linux__
	if (WSAStartup(MAKEWORD(2, 0), &wsaData)) {
		std::cout << "WSAStartup failed! aborting...";
		return;
	}
#endif
	int err=getaddrinfo(NULL,"45451",&hints,&res);
	if(err){
		printf("getaddrinfo error!:%d",err);
	}
	sock0 = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sock0 == INVALID_SOCKET) {
		printf("socket failed! aborting...\n");
		return;
	}
	int yes = 1;
	setsockopt(sock0, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes));
	if (bind(sock0, res->ai_addr, res->ai_addrlen)) {
		printf("bind failed! aborting...\n");
		return;
	}
	if (listen(sock0, 5)) {
		printf("listen failed! aborting...\n");
		return;
	}
}
othheloserver::~othheloserver() {
#ifndef __linux__
	WSACleanup();
#endif
	freeaddrinfo(res);
	end = 0;
	endthread.join();
	Games.clear();
}

void othheloserver::Run() {
	endthread = std::thread([this] {
		while (end == 0) {
			Games.remove_if([](std::shared_ptr<Game> game) {return game->ended == 1; });
			AutoGames.remove_if([](std::shared_ptr<Game> game) {return game->ended == 1; });
		}
		});
	while (end == 0) {
		len = sizeof(client);
		sock = accept(sock0, (struct sockaddr*) & client, &len);
		if (sock == INVALID_SOCKET) {
			printf("accept failed! aborting...\n");
			end = 0;
			continue;
		}
		thread = std::thread([this] {
			SOCKET thsock;
			thsock = sock;
			char data[128] = { 0 };
			std::string ret;
			if (recv(thsock, data, 128,0) <= 0) {
				std::cerr << "recv error" << std::endl;
				closesocket(thsock);
				return;
			}
			ret = data;
			std::vector<std::string> temp = spritstring(ret);
			if (!temp[0].compare("ROOM")) {
				if (ret.find("PASSWORD") != std::string::npos) Games.emplace_back(new Game(thsock, std::stoi(temp[1]), std::stoi(temp[2]), temp[4]));
				else Games.emplace_back(new Game(thsock, std::stoi(temp[1]), std::stoi(temp[2])));
			} else if (!temp[0].compare("LOGIN")) {
				int logined = 0;
				for (std::shared_ptr<Game> game : Games) {
					if (!game->room.compare(temp[1])) {
						if (temp.size() > 2 && !temp[2].compare("PASSWORD")) {
							game->login(thsock, temp[3]);
							logined = 1;
						} else {
							game->login(thsock);
							logined = 1;
						}
					}
				}
				if (logined == 0) {
					std::string req = "FAILED";
					if (send(thsock, req.c_str(), req.length() + 1,0) <= 0) {
						std::cerr << "send error" << std::endl;
					}
					closesocket(thsock);
				}
			} else if (!temp[0].compare("AUTO")) {
				if (AutoGames.size() == 0 || AutoGames.back()->started == 1) {
					if (send(thsock, "HOST", 5,0) <= 0) {
						std::cerr << "send error" << std::endl;
					}
				} else {
					std::string lastroom = "GUEST " + AutoGames.back()->room;
					if (send(thsock, lastroom.c_str(), lastroom.length() + 1,0) <= 0) {
						std::cerr << "send error" << std::endl;
					}
				}
				if (recv(thsock, data, 128,0) <= 0) {
					std::cerr << "recv error" << std::endl;
					closesocket(thsock);
					return;
				}
				ret = data;
				temp = spritstring(ret);
				if (!temp[0].compare("ROOM")) {
					AutoGames.emplace_back(new Game(thsock, std::stoi(temp[1]), std::stoi(temp[2])));
					return;
				} else if (!temp[0].compare("LOGIN")) {
					int logined = 0;
					for (std::shared_ptr<Game> game : AutoGames) {
						if (!game->room.compare(temp[1])) {
							game->login(thsock);
							logined = 1;
							break;
						}
					}
					if (logined == 0) {
						std::string req = "FAILED";
						if (send(thsock, req.c_str(), req.length() + 1,0) <= 0) {
							std::cerr << "send error0" << std::endl;
						}
						closesocket(thsock);
						return;
					}
				} else {
					std::string req = "FAILED";
					if (send(thsock, req.c_str(), req.length() + 1,0) <= 0) {
						std::cerr << "send error" << std::endl;
					}
					closesocket(thsock);
				}
			}
			});
		thread.detach();

	}
}
