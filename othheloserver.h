#pragma once
#include <memory>
#include <list>
#include <vector>
#include <sstream>
#include <iostream>
#include "game.h"
#include <errno.h>  
#include <openssl/bio.h>
#include <openssl/err.h>
#include <atomic>

class othheloserver{
	public:
	othheloserver();
	~othheloserver();
	void Run();
	int connected;
	std::atomic<int> hgselect;
	std::list<std::shared_ptr<Game>> Games;
	std::list<std::shared_ptr<Game>> AutoGames;
	BIO *sock0;
	int end;
	int len;
	BIO* sock;
	std::thread thread,endthread;
};