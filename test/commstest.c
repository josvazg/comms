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

int client() {
	Error err;
	char buffer[MAXBUF];
	int w,r,total=0;
	Address addr,raddr;
	char* req="GET / HTTP/1.0\nHost: www.someserver.com\n\n";
	Conn c=NULL;
	
	// Initialization forced for Windows compatibility
	commsInit(err);
	dieOnError("CLIENT",&err);

	// TCP Connection
	c=connDial("tcp", ":1080", err);
	dieOnError("CLIENT",&err);

	// What is the final Local & Remote Address?
	connAddress(c,addr);
	connRemoteAddress(c,raddr);
	printf("CLIENT:%s connected to %s\n",addr,raddr);

	// Write / send some data (a request)
	w=connWrite(c,req,strlen(req));
	dieOnError("CLIENT",connError(c));
	printf("CLIENT: written %d of %d requests bytes\n",w,(int)strlen(req));

	// Read /recv data (the response)
	for(r=0;(r=connRead(c,buffer,MAXBUF))>0;) {
		buffer[r]='\0';
		printf("%s",buffer);
		total+=r;
	}
	printf("CLIENT: %dbytes readed total!\n",total);

    // Close the conn(ection)
	if(connClose(c)) {
		dieOnError("CLIENT",connError(c));
	}
	return 0;
}

void serve(Conn c) {
	char buffer[MAXBUF];
	int r=0,w=0;
	Address raddr;
	connAddress(c,raddr);
	printf("SERVER: Connection from %s\n",raddr);

	// Read /recv data (the response)
	if((r=connRead(c,buffer,MAXBUF))>0) {
		buffer[r]='\0';
		printf("%s",buffer);
	}
	printf("SERVER: %dbytes readed!\n",r);
	// Echo reply
	w=connWrite(c,buffer,r);
	dieOnError("SERVER",connError(c));
	printf("SERVER: written %d of %d requests bytes\n",w,r);

    // Close the conn(ection)
	if(connClose(c)) {
		fprintf(stderr,"%s",(char*)connError(c));
		return;
	}
}

int server() {
	Error err;
	Address addr;
	Serv s=NULL;
	Conn c=NULL;
	
	// Initialization forced for Windows compatibility
	commsInit(err);
	dieOnError("SERVER",&err);

	// TCP Server
	s=servNew("tcp", ":1080", err);
	dieOnError("SERVER",&err);

	// What is the final Listen Address?
	servAddress(s,addr);
	printf("SERVER: Listens on %s\n",addr);

	// Listen for a connection
	c=servListen(s);
	dieOnError("SERVER",servError(s));
	serve(c);

    // Close the server
	if(servClose(s)) {
		dieOnError("SERVER",servError(s));
	}
	return 0;
}

int main(int argc, char* argv[]) {
	if(argc>1 && (argv[1][0]=='s' || argv[1][0]=='S')) {
		return server();
	} else {
		return client();
	}
}
