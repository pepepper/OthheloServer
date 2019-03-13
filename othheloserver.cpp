#include "othheloserver.h"

std::vector<std::string> spritstring(std::string str){
	std::vector<std::string> ret;
	std::string temp;
	std::stringstream ss{str};
	while(std::getline(ss, temp, ' ')){
		ret.push_back(temp);
	}
	return ret;
}
#ifdef Linux_System
othheloserver::othheloserver():connected(0), end(0){
	sock0 = socket(AF_INET, SOCK_STREAM, 0);

	addr.sin_family = AF_INET;
	addr.sin_port = htons(45451);
	addr.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(sock0, (struct sockaddr *)&addr, sizeof(addr));

	listen(sock0, 5);
	gamedecon = std::thread([this]{
		while(end){
			Games.remove_if([](std::shared_ptr<Game> game){return game->ended; });
		}

							});
}
othheloserver::~othheloserver(){
	gamedecon.join();
	Games.clear();
}
void othheloserver::Run(){
	while(1){
		len = sizeof(client);
		sock = accept(sock0, (struct sockaddr *)&client, &len);
		thread = std::thread([this]{
			char data[1024] = {0};
			std::string ret;
			recv(sock, data, 1024, 0);
			ret = data;
			std::vector<std::string> temp = spritstring(ret);
			if(!temp[0].compare("ROOM")){
				Games.emplace_back(new Game(sock, std::stoi(temp[1]), std::stoi(temp[2])));
			} else if(!temp[0].compare("LOGIN")){
				for(std::shared_ptr<Game> game : Games){
					if(!game->room.compare(temp[1])){
						if(temp.size() > 2 && !temp[2].compare("PASSWORD")){
							game->login(&sock, temp[3]);
						} else{
							game->login(&sock);
						}
					} else{
						std::string req = "FAILED";
						send(sock, req.c_str(), req.length() + 1, 0);
						close(sock);
					}
				}
			}});
		thread.detach();
	}
}
#else
othheloserver::othheloserver(): connected(0),end(0){
	WSAStartup(MAKEWORD(2, 0), &wsaData);
	sock0 = socket(AF_INET, SOCK_STREAM, 0);
	if(sock0 == INVALID_SOCKET){
		printf("socket : %d\n", WSAGetLastError());
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(45451);
	addr.sin_addr.S_un.S_addr = INADDR_ANY;

	if(bind(sock0, (struct sockaddr *)&addr, sizeof(addr)) != 0){
		printf("bind : %d\n", WSAGetLastError());
	}

	if(listen(sock0, 5) != 0){
		printf("listen : %d\n", WSAGetLastError());
	}
	gamedecon = std::thread([this]{
		while(end){
			Games.remove_if([](std::shared_ptr<Game> game){return game->ended; });
		}

							});
}
othheloserver::~othheloserver(){
	gamedecon.join();
	Games.clear();
	WSACleanup();
}


void othheloserver::Run(){
	while(1){
		len = sizeof(client);
		sock = accept(sock0, (struct sockaddr *)&client, &len);
		if(sock == INVALID_SOCKET){
			printf("accept : %d\n", WSAGetLastError());
		}
		thread = std::thread([this]{
			char data[1024] = {0};
			std::string ret;
			recv(sock, data, 1024, 0);
			ret = data;
			std::vector<std::string> temp = spritstring(ret);
			if(!temp[0].compare("ROOM")){
				Games.emplace_back(new Game(sock, std::stoi(temp[1]), std::stoi(temp[2])));
			} else if(!temp[0].compare("LOGIN")){
				for(std::shared_ptr<Game> game : Games){
					if(!game->room.compare(temp[1])){
						if(temp.size() > 2 && !temp[2].compare("PASSWORD")){
							game->login(&sock, temp[3]);
						} else{
							game->login(&sock);
						}
					} else{
						std::string req = "FAILED";
						send(sock, req.c_str(), req.length() + 1, 0);
						closesocket(sock);
					}
				}
			}});
		thread.detach();
	}
}

#endif // Linux_System