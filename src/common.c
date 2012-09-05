#include <stdio.h>
#include "comms.h"
#include "defs.h"

#ifdef _WIN32
int commsInit(Error err) {
	int r=0;
	err[0]='\0';
  	memset(&wsaData, 0, sizeof(wsaData));
  	r=WSAStartup(0x0202, &wsaData);
  	if(r) {
  		newError(err,"WSAStartup failed with error: %d\n", r);
  	}
  	return r;
}
const char* errortext(int errcode, Error e) {
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,0,errcode,0,e,MAX_ERROR_SIZE,0);
	return (const char*)e;
}
const char* addr2text(LPSOCKADDR src, char *dst, int size) {
	int len=addrSize(src->sa_family);
	if(!WSAAddressToString(src,len,NULL,dst,&size)) {
		return dst;
	}
	return NULL;
} 
#else
int closesocket(int socket) { return close(socket); }
int commsInit(Error err) {
	err[0]='\0';
	return 0;
}
const char* errortext(Error e) {
	e[0]='\0';
	if(strerror_r(errno,e,MAX_ERROR_SIZE)!=0) {
		return NULL;
	}
	return e;
}
const char* addr2text(struct sockaddr* src, char *dst, socklen_t size) {
	int pos=0;
	int port=-1;
	if(src->sa_family==AF_INET) {
		struct sockaddr_in *saddr=(struct sockaddr_in*)src;
		inet_ntop(AF_INET, &saddr->sin_addr, dst, size);
		port=ntohs(saddr->sin_port);
		pos=strlen(dst);
		snprintf(&dst[pos],MAX_ADDR_SIZE-pos,":%d",port);
	} else if(src->sa_family==AF_INET6) {
		Address a;
		struct sockaddr_in6 *saddr=(struct sockaddr_in6*)src;
		inet_ntop(AF_INET6, &saddr->sin6_addr, a, size);
		port=ntohs(saddr->sin6_port);
		snprintf(dst,MAX_ADDR_SIZE-pos,"[%s]:%d",a,port);
	} else {
		return NULL;
	}
	return dst;
}
#endif

// sizeOfType gives the size of type
int sizeOfType(int type) {
	switch(type) {
		case SOCKSTREAM_TYPE:
		case SOCKDGRAM_TYPE:
			return sizeof(struct Conn_S);
		case SERVSOCKSTREAM_TYPE:
			return sizeof(struct Serv_S);
		case FILE_TYPE:
			return sizeof(struct IO_S);
	}
	return 0;
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

// addrSize returns the sockaddress size
int addrSize(int af) {
	int len=sizeof(struct sockaddr_in);
	if(af==AF_INET6) {
		len=sizeof(struct sockaddr_in6);
	}
	return len;
}

// writeAddress saddr as text
void writeAddress(Address addr, struct sockaddr* saddr) {
	memset(addr,0,MAX_ADDR_SIZE);
	addr2text(saddr,addr,MAX_ADDR_SIZE);
}

// solveAddress return the resolver address socket for the given Address text addr
struct addrinfo* solveAddress(char* addr, Error err, int type, char* defaddr) {
	struct addrinfo hints;
	struct addrinfo* result;
	char *node, *service;
	int error,index;
	if(strlen(addr)==0) {
		newError(err,"Address is empty!\n");
		return NULL;
	}
	index=last(addr,':');
	if(index<0) {
		newError(err,"Invalid Address expected 'node:service' but got '%s'!\n",addr);
		return NULL;
	} else if(index==0) {
		node=defaddr;
	} else {
		node=alloca(index+2); // clone the node part from the begining to the ':' index
		memset(node,0,index+2);
		memcpy(node,addr,index);
	}
	service=&addr[index+1]; // service points to the port/service end of addr
	memset(&hints, 0, sizeof(struct addrinfo));
	// Name solving...
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = type; /* Stream socket */
    hints.ai_flags = 0;
    hints.ai_protocol = 0;          /* Any protocol */
	error=getaddrinfo(node, service, &hints, &result);
    if (error) {
        newError(err,"Getaddrinfo: %s\n", gai_strerror(error));
        return NULL;
    }
	return result;
}

// sockAddress fills addr with the socket's local address
void sockAddress(Sock sock, Address addr) {
	struct sockaddr *saddr=NULL;
	int len=addrSize(sock->ver);
	saddr=alloca(len);
	if(saddr==NULL) {
		newError(sock->e,"Can't allocate space for socket address!");
		return;
	}
	if(getsockname(sock->s,saddr,&len)) {
		Error e;
		newError(sock->e,"Getsockname: %s",ERRDESC(e));
		return;
	}
	writeAddress(addr,saddr);
}
