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
int closesocket(int socket);
int commsInit(Error err);
const char *addr2text(struct sockaddr* src, char *dst, socklen_t size);
#endif

// Connection Structure
struct Conn_S {
	int type;
	int ver;
	int s;
	Error e;
	Address remote;
};

/* Common helper functions */
char* errdesc();
int onError(Error err);
int last(char* s, char c);
int addrSize(int af);
void writeAddress(Address addr, struct sockaddr* saddr);
struct addrinfo* solveAddress(char* addr, Error err, int type, char* defaddr);
int closeEndPoint(int s, Error err, void* ptr,int size);