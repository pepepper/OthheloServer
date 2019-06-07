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
othheloserver::othheloserver() :connected(0), end(0), hgselect(0) {
	int yes = 1;
	ERR_load_BIO_strings();
	sock0 = BIO_new_accept("45451");
	BIO_set_bind_mode(sock0, BIO_BIND_REUSEADDR);
	if (BIO_do_accept(sock0) <= 0) {
		std::cerr << "accept error";
		ERR_print_errors_fp(stderr);
		return;
	}
}
othheloserver::~othheloserver() {
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
		if (BIO_do_accept(sock0) <= 0) {
			std::cerr << "accept error";
			ERR_print_errors_fp(stderr);
			return;
		}
		sock = BIO_pop(sock0);

		thread = std::thread([this] {
			BIO *thsock;
			thsock = sock;
			char data[128] = { 0 };
			std::string ret;
			if (BIO_read(thsock, data, 128) <= 0) {
				std::cerr << "recv error" << std::endl;
				ERR_print_errors_fp(stderr);
				BIO_free_all(thsock);
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
					if (BIO_write(thsock, req.c_str(), req.length() + 1) <= 0) {
						std::cerr << "send error:" << std::endl;
						ERR_print_errors_fp(stderr);
					}
					BIO_free_all(thsock);
				}
			} else if (!temp[0].compare("AUTO")) {
				if (hgselect == 0) {
					if (BIO_write(sock, "HOST", 5) <= 0) {
						std::cerr << "send error:" << std::endl;
						ERR_print_errors_fp(stderr);
					}
				} else {
					std::string lastroom = "GUEST " + AutoGames.back()->room;
					if (BIO_write(sock, lastroom.c_str(), lastroom.length() + 1) <= 0) {
						std::cerr << "send error:" << std::endl;
						ERR_print_errors_fp(stderr);
					}
				}
				if (BIO_read(thsock, data, 128) <= 0) {
					std::cerr << "recv error" << std::endl;
					ERR_print_errors_fp(stderr);
					BIO_free_all(thsock);
					return;
				}
				ret = data;
				temp = spritstring(ret);
				if (!temp[0].compare("ROOM")) {
					Games.emplace_back(new Game(sock, std::stoi(temp[1]), std::stoi(temp[2])));
					hgselect = !hgselect;
					return;
				} else if (!temp[0].compare("LOGIN")) {
					int logined = 0;
					for (std::shared_ptr<Game> game : Games) {
						if (!game->room.compare(temp[1])) {
							game->login(sock);
							logined = 1;
							break;
						}
					}
					if (logined == 0) {
						std::string req = "FAILED";
						if (BIO_write(sock, req.c_str(), req.length() + 1) <= 0) {
							std::cerr << "send error:" << std::endl;
							ERR_print_errors_fp(stderr);
						}
						BIO_free_all(sock);
						return;
					}
					hgselect = !hgselect;
				} else {
					std::string req = "FAILED";
					if (BIO_write(sock, req.c_str(), req.length() + 1) <= 0) {
						std::cerr << "send error:" << std::endl;
						ERR_print_errors_fp(stderr);
					}
					BIO_free_all(sock);
				}
			}
			});
		thread.detach();

	}
}
