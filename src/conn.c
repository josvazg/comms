#include "comms.h"
#include "defs.h"

// dialTcp connects to a TCP socket address 'addr' or fills err
void dialTcp(Conn conn, char* addr, Error err) {
    struct addrinfo *results, *rp;
	results=solveAddress(addr,err,SOCK_STREAM,"localhost");
	if(onError(err)) {
		return;
	}
	if(results==NULL || results->ai_addr==NULL) {
  		newError(err,"Unresolved address: %s",addr);
  		return;
  	}
	/* getaddrinfo() returns a list of address structures.
       Try each address until we successfully connect(2).
       If socket(2) (or connect(2)) fails, we (close the socket and) try the next address. */

	for (rp = results; rp != NULL; rp = rp->ai_next) {
		conn->s = socket(rp->ai_family, rp->ai_socktype,rp->ai_protocol);
        if (conn->s == -1) {
            continue;
		}
		if (connect(conn->s, rp->ai_addr, rp->ai_addrlen) != -1) {
            break;                  /* Success */
		}
       	close(conn->s);
	}
	if (rp == NULL) {               /* No address succeeded */
		conn->s=0;
        newError(err,"Could not connect with %s\n",addr);
		return;
    }
	// Otherwise we are connected
	conn->type=rp->ai_socktype;
	conn->ver=rp->ai_family;
	writeAddress(conn->remote,rp->ai_addr);
	freeaddrinfo(results);
}

// connDial connects or prepares a communication on a network 'net' to address 'addr'
// On any error, connDial returns NULL and err will have some error message filled in 
Conn connDial(char* net, char* addr, Error err) {
	Conn conn;
	err[0]='\0';
	// Create struct
	conn=malloc(sizeof(struct Conn_S));
	if(conn==NULL) {
		newError(err,"Could not create Conn!\n");
		return NULL;
	}
	// clear/zero it
	memset(conn,0,sizeof(struct Conn_S));	
    // dial on the given 'net' network
	if(strcmp(net,"tcp")==0) { // tcp socket
		dialTcp(conn,addr,err);
	} else {
		newError(err,"Unknown net '%s'!\n",net);
	}
	if(onError(err)) {
		free(conn);
		return NULL;
	}
	return conn;
}

// connError returns the latest Conn's Error
// use onError(connError(conn)) to test for conn(ection) errors
Error* connError(Conn conn) {	
	return (Error*)conn->e;
}

// Fills addr with the local address
// On any error the address is empty and Conn's Error is set
void connAddress(Conn conn, Address addr) {
	sockAddress((CommonSocket)conn,addr);
}

// Fills raddr with the remote connected (or last received data) address
// On any error the address is empty and Conn's Error is set
void connRemoteAddress(Conn conn, Address raddr) {
	snprintf(raddr,MAX_ADDR_SIZE,"%s",conn->remote);
	if(strlen(raddr)==0) {
		newError(conn->e,"Can't get a Remote Address!");
	}
}

// connRead reads contents from conn to the given buffer buf (at most size bytes) and
// returns the number of bytes read OR -1 and Conn's Error is set
int connRead(Conn conn, char* buf, int size) {
	conn->e[0]='\0';
	if(conn->type==SOCK_STREAM) {
		// WinXP (at least) doesn't support read on a socket, so we use recv instead
		int readed=recv(conn->s, buf, size,0);
		if(readed<0) {
			Error e;
			newError(conn->e,"ConnRead error %s\n",ERRDESC(e));
		}
		return readed;
	}
	newError(conn->e,"Unsupported Conn(ection) state!\n");
	return -1;
}

// connWrite writes contents from the buf buffer to Conn and
// returns the number of bytes written OR -1 and Conn's Error is set
int connWrite(Conn conn, char* buf, int size){
	conn->e[0]='\0';
	if(conn->type==SOCK_STREAM) {
		// WinXP (at least) doesn't support write on a socket, so we use send instead
		int written=send(conn->s, buf, size,0);
		if(written<0) {
			Error e;
			newError(conn->e,"ConnWrite error %s\n",ERRDESC(e));
		}
		return written;
	}
	newError(conn->e,"Unsupported Conn(ection) state!\n");
	return -1;
}

// connClose closes the Connection/Stream
// On success it should return 0 and conn is no longer points to valid data, SO DON'T USE IT AGAIN!
// On failure it returns a non zero value and Conn's Error is set
int connClose(Conn conn) {
	return sockClose((CommonSocket)conn,sizeof(struct Conn_S));
}


