#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/comms.h"

#define MAXBUF 1024

void dieOnError(char *who, Error* e) {
	if(strlen((char*)e)>0) {
		fprintf(stderr,"%s: %s\n",who, (char*)e);
		exit(-1);
	}
}

int client(char* type) {
	Error err;
	char buffer[MAXBUF];
	int w,r,total=0;
	Address addr,raddr;
	char* req="GET / HTTP/1.0\nHost: www.someserver.com\n\n";
	Conn c=NULL;
	char prefix[64];
	snprintf(prefix,64,"Client%s",type);
	
	// Initialization forced for Windows compatibility
	commsInit(err);
	dieOnError(prefix,&err);

	// TCP Connection
	c=connDial(type, ":1080", err);
	dieOnError(prefix,&err);

	// What is the final Local & Remote Address?
	connAddress(c,addr);
	connRemoteAddress(c,raddr);
	printf("%s:%s connected to %s\n",prefix,addr,raddr);

	// Write / send some data (a request)
	w=connWrite(c,req,strlen(req));
	dieOnError(prefix,connError(c));
	printf("%s: written %d of %d requests bytes\n",prefix,w,(int)strlen(req));

	// Read /recv data (the response)
	for(r=0;(r=connRead(c,buffer,MAXBUF))>0;) {
		buffer[r]='\0';
		printf("%s",buffer);
		total+=r;
	}
	printf("%s: %dbytes readed total!\n",prefix,total);

    // Close the conn(ection)
	if(connClose(c)) {
		dieOnError(prefix,connError(c));
	}
	return 0;
}

void serve(Conn c, char* type) {
	char buffer[MAXBUF];
	int r=0,w=0;
	Address raddr;
	char prefix[64];
	snprintf(prefix,64,"Server%s",type);

	connAddress(c,raddr);
	printf("%s: Connection from %s\n",prefix, raddr);

	// Read /recv data (the response)
	if((r=connRead(c,buffer,MAXBUF))>0) {
		buffer[r]='\0';
		printf("%s",buffer);
	}
	printf("%s: %dbytes readed!\n",prefix,r);
	// Echo reply
	w=connWrite(c,buffer,r);
	dieOnError(prefix,connError(c));
	printf("%s: written %d of %d requests bytes\n",prefix,w,r);

    // Close the conn(ection)
	if(connClose(c)) {
		dieOnError(prefix,connError(c));
	}
}

int server(char *type) {
	Error err;
	Address addr;
	Serv s=NULL;
	Conn c=NULL;
	char prefix[64];
	snprintf(prefix,64,"Server%s",type);
	
	// Initialization forced for Windows compatibility
	commsInit(err);
	dieOnError(prefix,&err);

	// TCP Server
	s=servNew(type, ":1080", err);
	dieOnError(prefix,&err);

	// What is the final Listen Address?
	servAddress(s,addr);
	printf("%s: Listens on %s\n",prefix,addr);

	// Listen for a connection
	c=servListen(s);
	dieOnError(prefix,servError(s));
	serve(c,type);

    // Close the server
	if(servClose(s)) {
		dieOnError(prefix,servError(s));
	}
	return 0;
}

int main(int argc, char* argv[]) {
	if(argc>1 && strcmp(argv[1],"udp")==0) {
		return client("udp");
	} else if(argc>1 && strcmp(argv[1],"serverudp")==0) {
		return server("udp");
	} else if(argc>1 && strcmp(argv[1],"servertcp")==0) {
		return server("tcp");
	} else {
		return client("tcp");
	}
}
