#include "game.h"
#include <chrono>
#include <iostream>
#include <sstream>
#include <vector>

Game::Game(SOCKET connection, int w, int h): host(connection), islocked(0), guest(-1), x(w), y(h),ended(0){
	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
	room = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(start.time_since_epoch()).count());
	std::string reply = "SUCCESS " + room;
	if(send(host, reply.c_str(), reply.length()+1, 0)==-1)std::cout<<"send error:"<<errno<<std::endl;

	memset(&socks,0,sizeof(socks));
	socks[0].fd=host;
	socks[0].events=POLLIN|POLLERR;

	thread = std::thread([this]{
		char data[32] = {0};
		std::string req;
		while(1){
			req.erase();

			poll(socks,2,-1);

			if(socks[0].revents&POLLIN){
				if(recv(host, data, 32, 0) == -1){
					std::cout<<"recv error:"<<errno<<std::endl;
					send(guest, "CLOSED", 7, 0);
					socks[0].fd=0;
					break;
				}
				req = data;
				if(send(guest, req.c_str(), req.length()+1, 0)==-1){
					if(send(host, "FAILED", 7, 0)==-1)std::cout<<"send error:"<<errno<<std::endl;
					std::cout<<"send error:"<<errno<<std::endl;
					socks[1].fd=0;
				}
								if(send(host, "SUCCESS", 8, 0)==-1)std::cout<<"send error:"<<errno<<std::endl;
			}

			if(socks[1].revents&POLLIN){
				if(recv(guest, data, 32, 0) == -1){
					std::cout<<"recv error:"<<errno<<std::endl;
					send(host, "CLOSED", 7, 0);
					socks[1].fd=0;
					break;
				}
				req = data;
				if(send(host, req.c_str(), req.length()+1, 0)==-1){
					if(send(guest, "FAILED", 7, 0)==-1)std::cout<<"send error:"<<errno<<std::endl;
				std::cout<<"send error:"<<errno<<std::endl;
				socks[0].fd=0;
				}
								if(send(guest, "SUCCESS", 8, 0)==-1)std::cout<<"send error:"<<errno<<std::endl;
			}

			if(!req.compare("CLOSED")){
				if(send(host, req.c_str(), req.length() + 1, 0)==-1)std::cout<<"send error:"<<errno<<std::endl;
				if(send(guest, req.c_str(), req.length() + 1, 0)==-1)std::cout<<"send error:"<<errno<<std::endl;
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
				if(send(host, reply.c_str(), reply.length()+1 + 1, 0)==-1)std::cout<<"send error:"<<errno<<std::endl;
			}
		}
		ended = 1;
						 });
	return;
}

#ifdef Linux_System
Game::~Game(){
	ended = 1;
	std::string req="CLOSED";
	if(send(host, req.c_str(), req.length() + 1, 0)==-1)std::cout<<"send error:"<<errno<<std::endl;
	if(send(guest, req.c_str(), req.length() + 1, 0)==-1)std::cout<<"send error:"<<errno<<std::endl;
	thread.join();
	close(host);
	close(guest);
}
#else
Game::~Game(){
	ended = 1;
	std::string req="CLOSED";
	if(send(host, req.c_str(), req.length() + 1, 0)==-1)std::cout<<"send error:"<<errno<<std::endl;
	if(send(guest, req.c_str(), req.length() + 1, 0)==-1)std::cout<<"send error:"<<errno<<std::endl;
	thread.join();
	closesocket(host);
	closesocket(guest);
	WSACleanup();
}
#endif

void Game::readyGame(){
	std::string reply = "READY";
	if(send(host, reply.c_str(), reply.length()+1, 0)==-1)std::cout<<"send error:"<<errno<<std::endl;
	if(send(guest, reply.c_str(), reply.length()+1, 0)==-1)std::cout<<"send error:"<<errno<<std::endl;
	return;
}

void Game::login(SOCKET *sock){
	if(islocked==0){
		guest = *sock;
		std::string reply = "SUCCESS " + std::to_string(x) + " " + std::to_string(y);
		if(send(guest, reply.c_str(), reply.length() + 1, 0)==-1)std::cout<<"send error:"<<errno<<std::endl;
		socks[1].fd=host;
		socks[1].events=POLLIN|POLLERR;
		readyGame();
	} else{
		std::string reply = "FAILED";
		if(send(*sock, reply.c_str(), reply.length() + 1, 0)==-1)std::cout<<"send error:"<<errno<<std::endl;
	}
	return;
}

void Game::login(SOCKET *sock, std::string pass){
	if(islocked==1 && !password.compare(pass)){
		guest = *sock;
		std::string reply = "SUCCESS " + std::to_string(x) + " " + std::to_string(y);
		if(send(guest, reply.c_str(), reply.length()+1, 0)==-1)std::cout<<"send error:"<<errno<<std::endl;
		socks[1].fd=host;
	socks[1].events=POLLIN|POLLERR;
		readyGame();
	} else{
		std::string reply = "FAILED";
		if(send(*sock, reply.c_str(), reply.length()+1, 0)==-1)std::cout<<"send error:"<<errno<<std::endl;
	}
	return;
}