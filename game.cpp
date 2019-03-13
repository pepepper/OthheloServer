#include "game.h"
#include <chrono>
#include <iostream>
#include <sstream>
#include <vector>

Game::Game(SOCKET connection, int w, int h): host(connection), islocked(0), guest(INVALID_SOCKET), x(w), y(h),ended(0){
	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
	room = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(start.time_since_epoch()).count());
	std::string reply = "SUCCESS " + room;
	send(host, reply.c_str(), reply.length()+1, 0);
	FD_ZERO(&socks);
	FD_SET(host, &socks);
	thread = std::thread([this]{
		fd_set fds;
		char data[32] = {0};
		std::string req;
		while(1){
			memcpy(&fds, &socks, sizeof(fd_set));
			req.erase();
			select(0, &fds, 0, 0, 0);
			if(FD_ISSET(host, &fds)){
				if(recv(host, data, 32, 0) == -1){
					send(guest, "CLOSED", 7, 0);
					break;
				}
				req += data;
				send(guest, req.c_str(), req.length()+1, 0);
			}

			if(FD_ISSET(guest, &fds)){
				if(recv(guest, data, 32, 0) == -1){
					send(host, "CLOSED", 7, 0);
					break;
				}
				req += data;
				send(host, req.c_str(), req.length()+1, 0);
			}
			if(!req.compare("CLOSED")){
				send(host, req.c_str(), req.length() + 1, 0);
				send(guest, req.c_str(), req.length() + 1, 0);
				break;
			}
			if(req.find("SETPASSWORD") != -1){
				std::vector<std::string> ret;
				std::string temp;
				std::stringstream ss{req};
				while(std::getline(ss, temp, ' ')){
					ret.push_back(temp);
				}
				islocked = 1;
				password = ret[1];
				std::string reply = "SUCCESS";
				send(host, reply.c_str(), reply.length()+1 + 1, 0);
			}
		}
		ended = 1;
						 });
	return;
}

#ifdef Linux_System
Game::~Game(){
	thread.join();
	close(host);
	close(guest);
}
#else
Game::~Game(){
	ended = 1;
	std::string req="CLOSED";
	send(host, req.c_str(), req.length() + 1, 0);
	send(guest, req.c_str(), req.length() + 1, 0);
	thread.join();
	closesocket(host);
	closesocket(guest);
	WSACleanup();
}
#endif

void Game::readyGame(){
	std::string reply = "READY";
	send(host, reply.c_str(), reply.length()+1, 0);
	send(guest, reply.c_str(), reply.length()+1, 0);
	return;
}

void Game::login(SOCKET *sock){
	if(!islocked){
		guest = *sock;
		std::string reply = "SUCCESS " + std::to_string(x) + " " + std::to_string(y);
		send(guest, reply.c_str(), reply.length() + 1, 0);
		FD_SET(guest, &socks);
		readyGame();
	} else{
		std::string reply = "FAILED";
		send(*sock, reply.c_str(), reply.length() + 1, 0);
	}
	return;
}

void Game::login(SOCKET *sock, std::string pass){
	if(islocked && !password.compare(pass)){
		guest = *sock;
		std::string reply = "SUCCESS " + std::to_string(x) + " " + std::to_string(y);
		send(guest, reply.c_str(), reply.length()+1, 0);
		FD_SET(guest, &socks);
		readyGame();
	} else{
		std::string reply = "FAILED";
		send(*sock, reply.c_str(), reply.length()+1, 0);
	}
	return;
}