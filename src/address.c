#include "comms.h"
#include "defs.h"

#ifdef _WIN32
const char* addr2text(LPSOCKADDR src, char *dst, int size) {
	int len=addrSize(src->sa_family);
	if(!WSAAddressToString(src,len,NULL,dst,&size)) {
		return dst;
	}
	return NULL;
} 
#else
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
		// clone the node part from the begining to the ':' index or between '[ and ']: for ipv6
		int start=0;
		int end=index;
		int len;
		if(addr[start]=='[') {
			start++;
		}
		if(addr[index-1]==']') {
			end--;
		}
		len=end-start;
		node=alloca(len+1); 
		memset(node,0,len+1);
		memcpy(node,&addr[start],len);
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