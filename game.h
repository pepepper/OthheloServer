#ifdef Linux_System
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <string.h>
#include <unistd.h>
typedef int SOCKET;
typedef struct pollfd WSAPOLLFD;
#else
#include <winsock2.h>
#define poll WSAPoll
#endif 
#include <thread>
#include <string>
#include <errno.h>  
class Game{
	public:
	Game(SOCKET connection, int w, int h);
	~Game();
	void login(SOCKET *sock);
	void login(SOCKET *sock, std::string pass);
	void readyGame();
	int islocked, ended;
	std::string room;
	private:
	int x, y;
	WSAPOLLFD socks[3];
	SOCKET host, guest;
	std::thread thread;
	std::string password;
};