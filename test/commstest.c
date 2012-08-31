#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/comms.h"

#define MAXBUF 1024

void dieOnError(Error* e) {
	if(strlen((char*)e)>0) {
		fprintf(stderr,"%s\n",(char*)e);
		exit(-1);
	}
}

int client() {
	Error err;
	char buffer[MAXBUF];
	int w,r,total=0;
	Address addr,raddr;
	char* req="GET / HTTP/1.0\nHost: www.google.com\n\n";
	Conn c=NULL;
	
	// Initialization forced for Windows compatibility
	commsInit(err);
	dieOnError(&err);

	// TCP Connection
	c=connDial("tcp", "www.google.com:80", err);
	dieOnError(&err);

	// What is the final Local & Remote Address?
	connAddress(c,addr);
	connRemoteAddress(c,raddr);
	printf("%s connected to %s\n",addr,raddr);

	// Write / send some data (a request)
	w=connWrite(c,req,strlen(req));
	dieOnError(connError(c));
	printf("written %d of %d requests bytes\n",w,(int)strlen(req));

	// Read /recv data (the response)
	for(r=0;(r=connRead(c,buffer,MAXBUF))>0;) {
		buffer[r]='\0';
		printf("%s",buffer);
		total+=r;
	}
	printf("%dbytes readed total!\n",total);

    // Close the conn(ection)
	if(connClose(c)) {
		fprintf(stderr,"%s",(char*)connError(c));
		return -1;
	}
	return 0;
}

void serve(Conn c) {
	char buffer[MAXBUF];
	int r,total=0;
	Address raddr;
	connAddress(c,raddr);
	printf("Connection from %s\n",raddr);

	// Read /recv data (the response)
	for(r=0;(r=connRead(c,buffer,MAXBUF))>0;) {
		buffer[r]='\0';
		printf("%s",buffer);
		total+=r;
	}
	printf("%dbytes readed total!\n",total);

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
	dieOnError(&err);

	// TCP Server
	s=servNew("tcp", ":1080", err);
	dieOnError(&err);

	// What is the final Listen Address?
	servAddress(s,addr);
	printf("Listens on %s\n",addr);

	// Listen for a connection
	c=servListen(s);
	dieOnError(servError(s));
	serve(c);

    // Close the server
	if(servClose(s)) {
		fprintf(stderr,"%s",(char*)connError(c));
		return -1;
	}
	return 0;
}

int main(int argc, char* argv[]) {
	//client();
	server();
}