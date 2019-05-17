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
othheloserver::othheloserver():connected(0), end(0){
	int yes = 1;
	ERR_load_BIO_strings();
	sock0=BIO_new_accept("45451");
	BIO_set_bind_mode(sock0, BIO_BIND_REUSEADDR);
	if(BIO_do_accept(sock0)<=0){
		std::cerr<<"accept error";
		ERR_print_errors_fp(stderr);
		return;
	}
}
othheloserver::~othheloserver(){
	Games.clear();
}

void othheloserver::Run(){
	while(1){
		if(BIO_do_accept(sock0)<=0){
			std::cerr<<"accept error";
			ERR_print_errors_fp(stderr);
			return;
		}
		sock=BIO_pop(sock0);

		thread = std::thread([this]{
			BIO *thsock;
			thsock = sock;
			char data[128] = {0};
			std::string ret;
			if(BIO_read(thsock, data, 128) <= 0){
				std::cerr << "recv error" << std::endl;
				ERR_print_errors_fp(stderr);
				BIO_free_all(thsock);
				return;
			}
			ret = data;
			std::vector<std::string> temp = spritstring(ret);
			if(!temp[0].compare("ROOM")){
				if(ret.find("PASSWORD")!=std::string::npos) Games.emplace_back(new Game(thsock, std::stoi(temp[1]), std::stoi(temp[2]),temp[4]));
				else Games.emplace_back(new Game(thsock, std::stoi(temp[1]), std::stoi(temp[2])));
			} else if(!temp[0].compare("LOGIN")){

				for(std::shared_ptr<Game> game : Games){

					if(!game->room.compare(temp[1])){

						if(temp.size() > 2 && !temp[2].compare("PASSWORD"))	game->login(thsock, temp[3]);
						else game->login(thsock);

					} else{

						std::string req = "FAILED";

						if(BIO_write(thsock, req.c_str(), req.length() + 1) <= 0){
							std::cerr << "send error:"  << std::endl;
							ERR_print_errors_fp(stderr);
						}

						BIO_free_all(thsock);

					}
				} 
			}else{
						std::string req = "FAILED";
						if(BIO_write(thsock, req.c_str(), req.length() + 1) <= 0){
							std::cerr << "send error:"  << std::endl;
							ERR_print_errors_fp(stderr);
						}
						BIO_free_all(thsock);
			}
			});
		thread.detach();
		Games.remove_if([](std::shared_ptr<Game> game){return game->ended; });
	}
}
