#ifdef __linux__
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
typedef int SOCKET;
#define closesocket(x) close(x)
#else
#include <winsock2.h>
#endif

#include <string.h>
#include <thread>
#include <string>
#include <errno.h>  
class Game{
	public:
	Game(SOCKET connection, int w, int h);
	Game(SOCKET connection, int w, int h,std::string pass);
	~Game();
	void login(SOCKET sock);
	void login(SOCKET sock, std::string pass);
	void readyGame();
	int islocked,started, ended;
	std::string room;
	private:
	int x, y;
	SOCKET host, guest;
	std::thread hthread,gthread;
	std::string password;
	int write(SOCKET con, std::string s);
	int read(SOCKET con, char* data, int size);
};
