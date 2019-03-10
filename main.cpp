#include <memory>
#include <vector>
#include <sstream>
#include "game.h"
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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#else
#include <winsock2.h> //todo ended thread
int main(){
	int connected = 0;
	std::vector<std::shared_ptr<Game>> Games;
	WSADATA wsaData;
	SOCKET sock0;
	struct sockaddr_in addr;
	struct sockaddr_in client;
	int len;
	SOCKET sock;
	fd_set socks;
	struct timeval timeout = {0, 0};
	std::thread thread;

	if(WSAStartup(MAKEWORD(2, 0), &wsaData) != 0){
		return 1;
	}
	FD_ZERO(&socks);
	sock0 = socket(AF_INET, SOCK_STREAM, 0);
	if(sock0 == INVALID_SOCKET){
		printf("socket : %d\n", WSAGetLastError());
		return 1;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(45451);
	addr.sin_addr.S_un.S_addr = INADDR_ANY;

	if(bind(sock0, (struct sockaddr *)&addr, sizeof(addr)) != 0){
		printf("bind : %d\n", WSAGetLastError());
		return 1;
	}

	if(listen(sock0, 5) != 0){
		printf("listen : %d\n", WSAGetLastError());
		return 1;
	}

	while(1){
		len = sizeof(client);
		sock = accept(sock0, (struct sockaddr *)&client, &len);
		if(sock == INVALID_SOCKET){
			printf("accept : %d\n", WSAGetLastError());
			break;
		}
		thread = std::thread([&sock,&Games]{
			char data[1024] = {0};
			std::string ret;
			recv(sock, data, 1024, 0);
			ret = data;
			std::vector<std::string> temp = spritstring(ret);
			if(!temp[0].compare("ROOM")){
				Games.emplace_back(new Game(sock, std::stoi(temp[1]), std::stoi(temp[2])));
			} else if(!temp[0].compare("LOGIN")){
				if(!temp[1].compare("PASSWORD")){
					for(std::shared_ptr<Game> game : Games){
						if(!game->room.compare(temp[1])){
							game->login(&sock, temp[3]);
						}
					}
				} else{
					for(std::shared_ptr<Game> game : Games){
						if(!game->room.compare(temp[1])){
							game->login(&sock);
						}
					}
				}
			} else{
				closesocket(sock);
			}});
		thread.detach();
	}

	WSACleanup();
}
#endif