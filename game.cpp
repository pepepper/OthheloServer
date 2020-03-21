#include "game.h"
#include <chrono>
#include <iostream>
#include <sstream>
#include <vector>

int  Game::write(SOCKET con, std::string s) {
	if (send(con, s.c_str(), s.length() + 1,0) <= 0) {
		std::cerr << "send error at "<<room;
		return -1;
	}
	return 0;
}

int Game::read(SOCKET con, char* data, int size) {
	if (recv(con, data, size,0) <= 0) {
		std::cerr << "recv error at "<<room;
		return -1;
	}
	return 0;
}

Game::Game(SOCKET connection, int w, int h) : host(connection), islocked(0), x(w), y(h), started(0), ended(0) {
	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
	room = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(start.time_since_epoch()).count());
	std::string reply = "SUCCESS " + room;
	if (write(host, reply)) {
		closesocket(host);
		ended = 1;
	}
	hthread = std::thread([this] {
		char data[32] = { 0 };
		std::string req;
		try {
			while (!ended) {
				if (read(host, data, 32)) {//read host command
					if (started == 1)closesocket(guest);
					ended = 1;
					continue;
				} else {
					req = data;
					if (started = 1 && write(guest, req)) {//send command to guest
						closesocket(host);
						ended = 1;
					}
				}
			}
		} catch (...) {
			closesocket(host);
			if (started == 1)closesocket(guest);
		}
		});
	return;
}

Game::Game(SOCKET connection, int w, int h, std::string pass) : host(connection), islocked(1), x(w), y(h), started(0), ended(0), password(pass) {
	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
	room = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(start.time_since_epoch()).count());
	std::string reply = "SUCCESS " + room;
	if (write(host, reply)) {
		closesocket(host);
		ended = 1;
	}
	hthread = std::thread([this] {
		char data[32] = { 0 };
		std::string req;
		try {
			while (!ended) {
				if (read(host, data, 32)) {//read host command
					if (started == 1)closesocket(guest);
					ended = 1;
					continue;
				} else {
					req = data;
					if (started = 1 && write(guest, req)) {//send command to guest
						closesocket(host);
						ended = 1;
					}
				}
			}
		} catch (...) {
			closesocket(host);
			if (started == 1)closesocket(guest);
		}
		});
	return;
}

Game::~Game() {
	ended = 1;
	if (hthread.joinable())hthread.join();
	if (gthread.joinable())gthread.join();
	closesocket(host);
	if (started == 1)closesocket(guest);
}

void Game::readyGame() {
	started = 1;
	write(host, "READY");
	write(guest, "READY");
	gthread = std::thread([this] {
		char data[32] = { 0 };
		std::string req;
		try {
			while (!ended) {
				if (read(guest, data, 32)) {//read host command
					closesocket(host);
					ended = 1;
				}
				req = data;
				if (write(host, req)) {//send command to guest
					closesocket(guest);
					ended = 1;
				}
			}
		} catch (...) {
			closesocket(host);
			closesocket(guest);
		}
		});
	return;
}

void Game::login(SOCKET sock) {
	if (islocked == 0) {
		guest = sock;
		std::string reply = "SUCCESS " + std::to_string(x) + " " + std::to_string(y);
		if (write(guest, reply)) {
			closesocket(guest);
			return;
		}
		readyGame();
	} else {
		std::string reply = "FAILED";
		if (write(sock, reply)) {
			closesocket(guest);
		}
	}
	return;
}

void Game::login(SOCKET sock, std::string pass) {
	if (islocked == 1 && !password.compare(pass)) {
		guest = sock;
		std::string reply = "SUCCESS " + std::to_string(x) + " " + std::to_string(y);
		if (write(guest, reply)) {
			closesocket(guest);
			return;
		}
		readyGame();
	} else {
		std::string reply = "FAILED";
		if (write(sock, reply)) {
			closesocket(guest);
		}
	}
	return;
}
