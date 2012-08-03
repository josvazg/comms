#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "conn.h"

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #define MYERRNO WSAGetLastError()
  #define alloca _alloca
  #define snprintf _snprintf
  WSADATA wsaData;
void commsInit(Error err) {
	err[0]='\0';
  	memset(&wsaData, 0, sizeof(wsaData));
  	WSAStartup(0x0202, &wsaData);
}
#else
  #include <errno.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  //#include <netinet/in.h>
  //#include <arpa/inet.h>
  #include <netdb.h>
  #define MYERRNO errno
void closesocket(int socket) { close(socket); }
void commsInit(Error err) {
	err[0]='\0'; 
}
#endif

#define CONN_TCP 1

struct Conn_S{
	int type;
	int s;
	Error e;
	struct addrinfo remote;
};

// errdesc returns a crossplatform error description 
char* errdesc() {
	return strerror(MYERRNO);
}

// newError creates an error text 
void newError(Error err, const char* fmt, ...) {
	va_list argp;
	va_start(argp, fmt);
	vsnprintf(err,MAX_ERROR_SIZE,fmt,argp);
	va_end(argp);
}

// Is this on error?, returns 0 when there is no error and !0 otherwise
int onError(Error err) {
	return strlen((const char*)err)>0;
}

// last finds the index of the last ocurrence for target char or -1 if not found
int last(char* s, char c) {
	int i=strlen(s);
	for(;i>=0 && s[i]!=c;i--);
	return i;	
}

// dialTcp connects to a TCP socket address 'addr' or fills err
void dialTcp(Conn conn, char* addr, Error err) {
	char txt[512];
    struct addrinfo hints;
    struct addrinfo *result, *rp;
	char *node, *service;
	int error,index;

	node=addr;
	index=last(addr,':');	
	if(index<0) {
		newError(err,"Invalid Address expected 'node:service' but got '%s'!\n",addr);
		return;
	} else if(index==0) {
		node="127.0.0.1";
	}
	service=&addr[index+1]; // service points to the port/service end of addr
	node=alloca(index+2); // clone the node part from the begining to the ':' index
	memset(node,0,index+2);
	memcpy(node,addr,index);
	// Connect to addr
	memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
    hints.ai_flags = 0;
    hints.ai_protocol = 0;          /* Any protocol */
	error=getaddrinfo(node, service, &hints, &result);
    if (error) {
        newError(err,"Getaddrinfo: %s\n", gai_strerror(error));
        return;
    }

	/* getaddrinfo() returns a list of address structures.
       Try each address until we successfully connect(2).
       If socket(2) (or connect(2)) fails, we (close the socket and) try the next address. */

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		conn->s = socket(rp->ai_family, rp->ai_socktype,rp->ai_protocol);
        if (conn->s == -1) {
            continue;
		}
		if (connect(conn->s, rp->ai_addr, rp->ai_addrlen) != -1) {
            break;                  /* Success */
		}
       	close(conn->s);
	}
	if (rp == NULL) {               /* No address succeeded */
		conn->s=0;
        newError(err,"Could not connect with %s\n",addr);
		return;
    }
	// Otherwise we are connected
	conn->type=CONN_TCP;
	memcpy(&conn->remote,rp,sizeof(struct addrinfo));
	freeaddrinfo(result);
}

// connDial connects or prepares a communication on a network 'net' to address 'addr'
// On any error, connDial returns NULL and err will have some error message filled in 
Conn connDial(char* net, char* addr, Error err) {
	Conn conn;
	err[0]='\0';
	// Create struct
	conn=malloc(sizeof(struct Conn_S));
	if(conn==NULL) {
		newError(err,"Could not create Conn!\n");
		return NULL;
	}
	// clear/zero it
	memset(conn,0,sizeof(struct Conn_S));	
    // dial on the given 'net' network
	if(strcmp(net,"tcp")==0) { // tcp socket
		dialTcp(conn,addr,err);
	} else {
		newError(err,"Unknown net '%s'!\n",net);
	}
	if(onError(err)) {
		free(conn);
		return NULL;
	}
	return conn;
}

// connError returns the latest Conn's Error
// use onError(connError(conn)) to test for conn(ection) errors
Error* connError(Conn conn) {	
	return (Error*)conn->e;
}

// Fills raddr with the remote connected (or last received data) address
// On any error the address is empty and Conn's Error is set
void connRemoteAddress(Conn conn, Address raddr) {
	int pos=0;
	int port=-1;
	memset(raddr,0,MAX_ADDR_SIZE);
	if(conn->remote.ai_family==AF_INET) {
		struct sockaddr_in *addr=(struct sockaddr_in*)conn->remote.ai_addr;
		inet_ntop(AF_INET, &addr->sin_addr, raddr, conn->remote.ai_addrlen);
		port=ntohs(addr->sin_port);
	} else if(conn->remote.ai_family==AF_INET6) {
		struct sockaddr_in6 *addr=(struct sockaddr_in6*)conn->remote.ai_addr;
		inet_ntop(AF_INET6, &addr->sin6_addr, raddr, conn->remote.ai_addrlen);
		port=ntohs(addr->sin6_port);
	} else {
		newError(raddr, "Unsupported address type %d!",conn->remote.ai_family);
	}
	pos=strlen(raddr);
	snprintf(&raddr[pos],MAX_ADDR_SIZE-pos,":%d",port);
}

// connRead reads contents from conn to the given buffer buf (at most size bytes) and
// returns the number of bytes read OR -1 and Conn's Error is set
int connRead(Conn conn, char* buf, int size) {
	conn->e[0]='\0';
	if(conn->type==CONN_TCP) {
		int readed=read(conn->s, buf, size);
		if(readed<0) {
			newError(conn->e,"ConnRead error %s\n",errdesc());
		}
		return readed;
	}
	newError(conn->e,"Unsupported Conn(ection) state!\n");
	return -1;
}

// connWrite writes contents from the buf buffer to Conn and
// returns the number of bytes written OR -1 and Conn's Error is set
int connWrite(Conn conn, char* buf, int size){
	conn->e[0]='\0';
	if(conn->type==CONN_TCP) {
		int written=write(conn->s, buf, size);
		if(written<0) {
			newError(conn->e,"ConnWrite error %s\n",errdesc());
		}
		return written;
	}
	newError(conn->e,"Unsupported Conn(ection) state!\n");
	return -1;
}

// connClose closes the Connection/Stream
// On success it should return 0 and conn is no longer points to valid data, SO DON'T USE IT AGAIN!
// On failure it returns a non zero value and Conn's Error is set
int connClose(Conn conn) {
	conn->e[0]='\0';
	if(closesocket(conn->s)) {
		printf("close failed!\n");
		newError(conn->e,"Could not close socket %d!\n",conn->s);
		return !0;
	}
	memset(conn,0,sizeof(struct Conn_S));
	free(conn);
	return 0;
}


