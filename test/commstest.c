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

	// Connection
	c=connDial(type, "127.0.0.1:9980", err);
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
		printf("%s: receiving...\n",prefix);
		buffer[r]='\0';
		printf("%s",buffer);
		total+=r;
		if(strcmp(type,"udp")==0) {
			// Udp can't wait for the connection to close... cause there is NO connection!
			break;
		}
	}
	printf("%s: %dbytes readed total!\n",prefix,total);

    // Close the conn(ection)
	if(connClose(c)) {
		dieOnError(prefix,connError(c));
	}
	return 0;
}

void serve(Conn c) {
	char buffer[MAXBUF];
	int r=0,w=0;
	Address raddr;

	connAddress(c,raddr);
	printf("Servertcp: Connection from %s\n", raddr);

	// Read /recv data (the response)
	if((r=connRead(c,buffer,MAXBUF))>0) {
		printf("Servertcp: receiving...\n");
		buffer[r]='\0';
		printf("%s",buffer);
	}
	printf("Servertcp: %dbytes readed!\n",r);
	// Echo reply
	w=connWrite(c,buffer,r);
	dieOnError("Servertcp",connError(c));
	printf("Servertcp: written %d of %d requests bytes\n",w,r);

    // Close the conn(ection)
	if(connClose(c)) {
		dieOnError("Servertcp",connError(c));
	}
}

int server() {
	Error err;
	Address addr;
	Serv s=NULL;
	Conn c=NULL;

	// TCP Server
	s=servNew("tcp", ":9980", err);
	dieOnError("Servertcp",&err);

	// What is the final Listen Address?
	servAddress(s,addr);
	printf("Servertcp: Listens on %s\n",addr);

	// Accept a connection
	c=servAccept(s);
	dieOnError("Servertcp",servError(s));
	serve(c);

    // Close the server
	if(servClose(s)) {
		dieOnError("Servertcp",servError(s));
	}
	return 0;
}

int serverUdp() {
	Error err;
	Address addr;
	Conn c=NULL;

	// (Udp Server) listener connection
	c=connListenMsgs("udp", ":9980", err);
	dieOnError("Serverudp",&err);

	// What is the final Listen Address?
	connAddress(c,addr);
	printf("Serverudp: Listens on %s\n",addr);

	// Serve the connection
	{
		char buffer[MAXBUF];
		int r=0,w=0;
		Address raddr;
		// Read /recv data (the response)
		if((r=connReadFrom(c,raddr,buffer,MAXBUF))>0) {
			buffer[r]='\0';
			printf("Serverudp: receiving...\n");
			printf("%s",buffer);
		}
		printf("Serverudp: %dbytes readed!\n",r);
		// Echo reply
		w=connWriteTo(c,raddr,buffer,r);
		dieOnError("Serverudp",connError(c));
		printf("Serverudp: written %d of %d requests bytes\n",w,r);
	}

    // Close the listening connection
	if(connClose(c)) {
		dieOnError("Serverudp",connError(c));
	}
	return 0;
}

int main(int argc, char* argv[]) {
	Error err;
	char prefix[64]="commstest";
	if(argc>1) {
		snprintf(prefix,64,"commtests %s",argv[1]);
	}
	// Initialization forced for Windows compatibility
	commsInit(err);
	dieOnError(prefix,&err);

	if(argc>1 && strcmp(argv[1],"udp")==0) {
		return client("udp");
	} else if(argc>1 && strcmp(argv[1],"serverudp")==0) {
		return serverUdp();
	} else if(argc>1 && strcmp(argv[1],"servertcp")==0) {
		return server();
	} else {
		return client("tcp");
	}
}
