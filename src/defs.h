#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

void newError(Error err, const char* fmt, ...);

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #define MYERRNO WSAGetLastError()
  #define alloca _alloca
  #define snprintf _snprintf
  WSADATA wsaData;
int commsInit(Error err);
const char *addr2text(LPSOCKADDR src, char *dst, int size);
#else
  #include <errno.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <netdb.h>
  #define MYERRNO errno 
  #define error2string strerror_r
int closesocket(int socket);
int commsInit(Error err);
const char *addr2text(struct sockaddr* src, char *dst, socklen_t size);
#endif

#define ERRDESC(v) error2string(MYERRNO,v,MAX_ERROR_SIZE)

#define SOCKSTREAM_TYPE 1
#define SOCKDGRAM_TYPE 2
#define SERVSOCKSTREAM_TYPE 3
#define FILE_TYPE 4

// IO
struct IO_S {
  int type;
  int s;
  Error e;
};

typedef struct Sock_S * Sock;

// Socket
struct Sock_S {
  int type;
  int s;
  Error e;
  int ver;
};

// Connection
struct Conn_S {
	int type;
	int s;
	Error e;
  int ver;
	Address remote;
};

// Server
struct Serv_S{
  int type;
  int s;
  Error e;
  int ver;
};

/* Common helper functions */
int onError(Error err);
int last(char* s, char c);
int addrSize(int af);
void writeAddress(Address addr, struct sockaddr* saddr);
struct addrinfo* solveAddress(char* addr, Error err, int type, char* defaddr);
void sockAddress(Sock sock, Address addr);
