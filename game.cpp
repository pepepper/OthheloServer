#include "game.h"
#include <chrono>
#include <iostream>
#include <sstream>
#include <vector>

int  write(BIO *con, std::string s){
	if(BIO_write(con, s.c_str(), s.length() + 1) <= 0){
		std::cerr << "send error:" << std::endl;
		ERR_print_errors_fp(stderr);
		return -1;
	}
	return 0;
}

int read(BIO *con, char *data, int size){
	if(BIO_read(con, data, size) <= 0){
		std::cerr << "recv error:" << std::endl;
		ERR_print_errors_fp(stderr);
		return -1;
	}
	return 0;
}

Game::Game(BIO *connection, int w, int h): host(connection), islocked(0), x(w), y(h), ended(0){
	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
	room = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(start.time_since_epoch()).count());
	std::string reply = "SUCCESS " + room;
	if(write(host, reply)){
		BIO_free_all(host);
		ended = 1;
	}
	return;
}

Game::Game(BIO *connection, int w, int h,std::string pass): host(connection), islocked(1), x(w), y(h), ended(0),password(pass){
	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
	room = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(start.time_since_epoch()).count());
	std::string reply = "SUCCESS " + room;
	if(write(host, reply)){
		BIO_free_all(host);
		ended = 1;
	}
	return;
}

Game::~Game(){
	ended=1;
	hthread.join();
	gthread.join();
	BIO_shutdown_wr(host);
	BIO_shutdown_wr(guest);
	BIO_free_all(host);
	BIO_free_all(guest);
}

void Game::readyGame(){
	std::string reply = "READY";
	write(host, "READY");
	write(guest, "READY");
		hthread = std::thread([this]{
		char data[32] = {0};
		std::string req;
		while(!ended){
			if(read(host, data, 32)){//read host command
				break;
			}
			req = data;
			if(!req.compare("CLOSED")){//closed
				write(guest, req);
				break;
			} else{
				if(write(guest, req)){//send command to guest
					break;
				}
			}
		}
		ended = 1;
						 });
	gthread = std::thread([this]{
		char data[32] = {0};
		std::string req;
		while(1){
			if(read(guest, data, 32)){//read guest command
				break;
			}
			req = data;
			if(!req.compare("CLOSED")){//closed
				write(host, req);
				break;
			}else{
				if(write(host, req)){//send command to host
					break;
				}
			}
		}
		ended = 1;
						 });
	return;
}

void Game::login(BIO *sock){
	if(islocked == 0){
		guest = sock;
		std::string reply = "SUCCESS " + std::to_string(x) + " " + std::to_string(y);
		if(write(guest, reply)){
			BIO_free_all(guest);
			return;
		}
		readyGame();
	} else{
		std::string reply = "FAILED";
		if(write(sock, reply)){
			BIO_free_all(sock);
		}
	}
	return;
}

void Game::login(BIO *sock, std::string pass){
	if(islocked == 1 && !password.compare(pass)){
		guest = sock;
		std::string reply = "SUCCESS " + std::to_string(x) + " " + std::to_string(y);
		if(write(guest, reply)){
			BIO_free_all(guest);
			return;
		}
		readyGame();
	} else{
		std::string reply = "FAILED";
		if(write(sock, reply)){
			BIO_free_all(sock);
		}
	}
	return;
}