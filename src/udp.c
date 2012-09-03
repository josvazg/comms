#include "comms.h"
#include "defs.h"

// dialUdp makes an UDP socket with a default remote address 'addr' or fills err
void dialUdp(Conn conn, char* addr, Error err) {
    struct addrinfo *result;
	result=solveAddress(addr,err,SOCK_DGRAM,"localhost");
	if(onError(err)) {
		return;
	}
	if(result==NULL || result->ai_addr==NULL) {
  		newError(err,"Unresolved address: %s",addr);
  		return;
  	}
	// Otherwise we are connected
	conn->type=SOCKDGRAM_TYPE;
	conn->ver=result->ai_family;
	writeAddress(conn->remote,result->ai_addr);
	freeaddrinfo(result);
}

// serverUdp binds to a UDP server socket address 'addr' or fills err
void serverUdp(Serv serv, char* addr, Error err) {
	struct addrinfo *ainfo;
 	struct sockaddr* saddr;
	int sockfd;
 	int reuseaddr=1;
  	// SOCKET
  	sockfd=socket(PF_INET,SOCK_DGRAM, 0);
  	if(sockfd<0) {
  		Error e;
  		newError(err,"Socket: %s",ERRDESC(e));
  		return;
  	}
  	// BIND
  	ainfo=solveAddress(addr,err,SOCK_DGRAM,"0.0.0.0");
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
	serv->type=SOCKDGRAM_TYPE;
	serv->s=sockfd;
	serv->ver=saddr->sa_family;
	freeaddrinfo(ainfo);
}

// connReadFrom reads contents from a remote UDP source
int connReadFrom(Conn conn, Address from, char* buf, int size) {
	if(conn->type==SOCKDGRAM_TYPE) {
		int readed=0;
		struct sockaddr* saddr;
		int len=addrSize(conn->ver);
		saddr=(struct sockaddr*)alloca(len);
		if(saddr==NULL) {
			Error e;
			newError(conn->e,"Can't allocate remote address: %s\n",ERRDESC(e));
		}
		readed=recvfrom(conn->s, buf, size,0, saddr, &len);
		if(readed<0) {
			Error e;
			newError(conn->e,"Recvfrom error %s\n",ERRDESC(e));
		}
		writeAddress(from,saddr);
		return readed;
	} else {
		newError(conn->e,"Unsupported IO type, must be an UDP Datagram Socket!\n");
		return -1;
	}
}

// connWriteTo writes contents of the buf buffer to the given remote UDP address 'to'
int connWriteTo(Conn conn, Address to, char* buf, int size) {
	if(conn->type==SOCKDGRAM_TYPE) {
		int written=0;
		struct addrinfo *ainfo;
		ainfo=solveAddress(to,conn->e,SOCK_DGRAM,"localhost");
	  	if(onError(conn->e)) {
  			return;
	  	}
		if(ainfo==NULL || ainfo->ai_addr==NULL) {
  			newError(conn->e,"Unresolved address: %s",to);
  			return;
	  	}
		written=sendto(conn->s, buf, size, 0, ainfo->ai_addr, ainfo->ai_addrlen);
		if(written<0) {
			Error e;
			newError(conn->e,"Sendto error %s\n",ERRDESC(e));
		}
		freeaddrinfo(ainfo);
		return written;
	} else {
		newError(conn->e,"Unsupported IO type, must be an UDP Datagram Socket!\n");
		return -1;
	}
}
