#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/conn.h"

#define MAXBUF 1024

void dieOnError(Error* e) {
	if(strlen((char*)e)>0) {
		fprintf(stderr,"%s\n",(char*)e);
		exit(-1);
	}
}

int main(int argc, char* argv[]) {
	Error err;
	char buffer[MAXBUF];
	int w,r,total=0;
	Address raddr;
	char* req="GET / HTTP/1.0\nHost: www.google.com\n\n";
	Conn c=NULL;
	// Initialization forced by Windows compatibility
	commsInit(err);
	dieOnError(&err);
	// TCP Connection
	c=connDial("tcp", "www.google.com:80", err);
	dieOnError(&err);
	// What is the final Remote Address?
	connRemoteAddress(c,raddr);
	printf("Connected to %s\n",raddr);
	// Write / send some data (a request)
	printf("A\n");
	w=connWrite(c,req,strlen(req));
	printf("B\n");
	dieOnError(connError(c));
	printf("written %d of %d requests bytes\n",w,(int)strlen(req));
	// Read /recv some data (the response)
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
