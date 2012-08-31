#include "comms.h"
#include "defs.h"

#define LISTEN_QUEUE_SIZE 5

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
#else
  #include <errno.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <netdb.h>
#endif

struct Serv_S{
	int type;
	int s;
	Error e;
	int ver;
};

// serverTcp binds to a TCP server socket address 'addr' or fills err
void serverTcp(Serv serv, char* addr, Error err) {
	struct addrinfo *ainfo;
 	struct sockaddr* saddr;
	int sockfd;
 	int reuseaddr=1;

  	// SOCKET
  	sockfd=socket(PF_INET,SOCK_STREAM, 0);
  	if(sockfd<0) {
  		Error e;
  		newError(err,"Socket: %s",ERRDESC(e));
  		return;
  	}
  	// SO_REUSEADDR
  	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&reuseaddr, sizeof(reuseaddr))<0) {
  		Error e;
  		newError(err,"Setsockopt SO_REUSEADDR: %s",ERRDESC(e));
  		return;
  	}
  	// BIND
  	ainfo=solveAddress(addr,err,SOCK_STREAM,"0.0.0.0");
  	if(onError(err)) {
  		return;
  	}
  	if(ainfo==NULL || ainfo->ai_addr==NULL) {
  		newError(err,"Unresolved address: %s",addr);
  		return;
  	}
  	saddr=ainfo->ai_addr;
  	if(bind(sockfd, (struct sockaddr*)saddr, ainfo->ai_addrlen)<0) {
  		Error e;
  		newError(err,"Bind: %s",ERRDESC(e));
  		return;
	}
  	// LISTEN
  	if(listen(sockfd,LISTEN_QUEUE_SIZE)<0) {
  		Error e;
  		newError(err,"Listen: %s",ERRDESC(e));
  		return;
	}
	serv->s=sockfd;
	serv->type=SOCK_STREAM;
	serv->ver=saddr->sa_family;
	freeaddrinfo(ainfo);
}

// servNew creates a new server on a network 'net' and address 'addr'
// On any error, servNew returns NULL and err will have some error message filled in 
Serv servNew(char* net, char* addr, Error err) {
	Serv serv;
	err[0]='\0';
	// Create struct
	serv=malloc(sizeof(struct Serv_S));
	if(serv==NULL) {
		newError(err,"Could not create Serv!\n");
		return NULL;
	}
	// clear/zero it
	memset(serv,0,sizeof(struct Serv_S));	
    // create server on the given 'net' network
	if(strcmp(net,"tcp")==0) { // tcp socket
		serverTcp(serv,addr,err);
	} else {
		newError(err,"Unknown net '%s'!\n",net);
		return NULL;
	}
	if(onError(err)) {
		free(serv);
		return NULL;
	}
	return serv;
}

// servError returns the latest Serv's Error
// use onError(ServError(conn)) to test for server errors
Error* servError(Serv serv) {
	return (Error*)serv->e;
}

// servAddress fills addr where the server is listening on
// On any error, servListen returns NULL and Serv's Error is set
void servAddress(Serv serv, Address addr) {
	sockAddress((IO)serv,addr);
}

// servListen listens and returns any incomming connection to the given server
// On any error, servListen returns NULL and Serv's Error is set
Conn servListen(Serv serv) {
	struct sockaddr* saddr;
	Conn conn;
	int len;
	int sockfd;
	sockfd=accept(serv->s,NULL,NULL);
	if(sockfd<0) {
		Error e;
		newError(serv->e,"Accept: %s\n",ERRDESC(e));
		return NULL;
	}
	len=addrSize(serv->ver);
	saddr=alloca(len);
	if(saddr==NULL) {
		newError(serv->e,"Could not store connecting socket address!\n");
		return NULL;
	}
	if(getpeername(sockfd,saddr,&len)<0) {
		Error e;
		newError(serv->e,"Getpeername: %s\n",ERRDESC(e));
		return NULL;
	}
	conn=malloc(sizeof(struct Conn_S));
	if(conn==NULL) {
		newError(serv->e,"Could not store connecting Conn!\n");
		return NULL;
	}
	memset(conn,sizeof(struct Conn_S),0);
	conn->type=serv->type;
	conn->ver=serv->ver;
	conn->s=sockfd;
	writeAddress(conn->remote,saddr);
	return conn;
}

// servClose closes the Server
// On success it should return 0 and serv is no longer points to valid data, SO DON'T USE IT AGAIN!
// On failure it returns a non zero value and Serv's Error is set
int servClose(Serv serv) {
	return sockClose((IO)serv,sizeof(struct Serv_S));
}