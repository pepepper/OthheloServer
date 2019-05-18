#include "othheloserver.h"
int main(){
	std::unique_ptr<othheloserver> server;
	server.reset(new othheloserver());
	server->Run();
	server->end = 1;
}