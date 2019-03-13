#ifdef Linux_System
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#else
#include <winsock2.h>
#endif 
#include <thread>
#include <string>
class Game{
	public:
	Game(SOCKET connection,int w,int h);
	~Game();
	void login(SOCKET *sock);
	void login(SOCKET *sock, std::string pass);
	void readyGame();
	int islocked,ended;
	std::string room;
	private:
	int x, y;
	fd_set socks;
	SOCKET host, guest;
	std::thread thread;
	std::string password;
};