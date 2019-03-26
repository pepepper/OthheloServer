#include "game.h"
#include <chrono>
#include <iostream>
#include <sstream>
#include <vector>

int  write(BIO *con, std::string s){
	if(BIO_write(con, s.c_str(), s.length() + 1) <= 0){
		std::cerr << "send error:" << errno << std::endl;
		ERR_print_errors_fp(stderr);
		return -1;
	}
	return 0;
}

int read(BIO *con, char *data, int size){
	if(BIO_read(con, data, size) <= 0){
		std::cerr << "recv error:" << errno << std::endl;
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

	thread = std::thread([this]{
		char data[32] = {0};
		std::string req;
		while(1){
			if(read(host, data, 32)){//read host command
				write(guest, "CLOSED");
				ended = 1;
				break;
			}
			req = data;
			if(req.find("SETPASSWORD") != -1){//set password
				std::vector<std::string> ret;
				std::string temp;
				std::stringstream ss{req};
				while(std::getline(ss, temp, ' ')){
					ret.push_back(temp);
				}
				islocked = 1;
				password = ret[1];
				std::string reply = "SUCCESS";
				if(write(host, reply))ended = 1;
				continue;
			} else if(!req.compare("CLOSED")){//closed
				write(host, req);
				write(guest, req);
				ended = 1;
				break;
			} else{
				if(write(guest, req)){//send command to guest
					write(host, "FAILED");
					ended = 1;
					break;
				}
				if(read(guest, data, 32)){//recv "SUCCESS" from guest
					write(host, "CLOSED");
					ended = 1;
					break;
				}
				if(write(host, data)){//send "SUCCESS" to host
					ended = 1;
					break;
				}
			}

			if(read(guest, data, 32)){//read guest command
				write(host, "CLOSED");
				ended = 1;
				break;
			}
			req = data;
			if(!req.compare("CLOSED")){//closed
				write(host, req);
				write(guest, req);
				ended = 1;
				break;
			} else{
				if(write(host, req)){//send command to host
					write(guest, "FAILED");
					ended = 1;
					break;
				}
				if(read(host, data, 32)){//recv "SUCCESS" from host
					write(guest, "CLOSED");
					ended = 1;
					break;
				}
				if(write(guest, data)){//send "SUCCESS" to guest
					ended = 1;
					break;
				}
			}
		}
		ended = 1;
						 });
	return;
}

Game::~Game(){
	ended = 1;
	write(host, "CLOSED");
	write(guest, "CLOSED");
	thread.join();
	BIO_free_all(host);
	BIO_free_all(guest);
}

void Game::readyGame(){
	std::string reply = "READY";
	write(host, "READY");
	write(guest, "READY");
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