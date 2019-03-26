#include <openssl/bio.h>
#include <openssl/err.h>
#include <string.h>
#include <thread>
#include <string>
#include <errno.h>  
class Game{
	public:
	Game(BIO *connection, int w, int h);
	~Game();
	void login(BIO *sock);
	void login(BIO *sock, std::string pass);
	void readyGame();
	int islocked, ended;
	std::string room;
	private:
	int x, y;
	BIO *host, *guest;
	std::thread thread;
	std::string password;
};