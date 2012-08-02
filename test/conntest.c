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
	int w,r,total;
	Address raddr;
	char* req="GET / HTTP/1.0\nHost: www.google.com\n\n";
	Conn c=ConnDial("tcp", "www.google.com:www", err);
	dieOnError(&err);
	ConnRemoteAddress(c,raddr);
	printf("Connected to %s\n",raddr);
	w=ConnWrite(c,req,strlen(req));
	dieOnError(ConnError(c));
	printf("written %d of %d requests bytes\n",w,(int)strlen(req));
	
	for(r=0;(r=ConnRead(c,buffer,MAXBUF))>0;) {
		buffer[r]='\0';
		printf("%s",buffer);
		total+=r;
	}
	printf("%dbytes readed total!\n",total);

	if(ConnClose(c)) {
		fprintf(stderr,"%s",(char*)ConnError(c));
		return -1;
	}
	return 0;
}
