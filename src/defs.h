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

#define FILE_TYPE SOCK_ANY

// IO
struct IO_S {
  int type;
  int s;
  Error e;
};

// Connection Structure
struct Conn_S {
	int type;
	int s;
	Error e;
  int ver;
	Address remote;
};

/* Common helper functions */
int onError(Error err);
int last(char* s, char c);
int addrSize(int af);
void writeAddress(Address addr, struct sockaddr* saddr);
struct addrinfo* solveAddress(char* addr, Error err, int type, char* defaddr);
int sockClose(CommonSocket cs, int size);
void sockAddress(CommonSocket cs, Address addr);