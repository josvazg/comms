#include "comms.h"
#include "defs.h"

// ioError returns the latest IO Error
Error* ioError(IO io) {
	return (Error*)io->e;
}

// ioRead reads contents from io to the given buffer buf (at most size bytes)
int ioRead(IO io, char* buf, int size) {
	int readed=0;
	io->e[0]='\0';
	if(io->type==SOCKSTREAM_TYPE) {
		// WinXP (at least) doesn't support read on a socket, so we use recv instead
		readed=recv(io->s, buf, size,0);
	} else if(io->type==SOCKDGRAM_TYPE) {
		Address from;
		readed=connReadFrom((Conn)io,from,buf, size);
		if(readed>=0 && strcmp(from,((Conn)io)->remote)!=0) {
			newError(io->e,
				"Data from unexpected source %s, you should be using ReadFrom instead!",from);
		}
	} else if(io->type==FILE_TYPE) {
		readed=read(io->s, buf, size);
	} else {
		newError(io->e,"Unsupported IO type!\n");
		return -1;
	}
	if(readed<0) {
		Error e;
		newError(io->e,"Read error %s\n",ERRDESC(e));
	}
	return readed;
}

// ioWrite writes contents from the buf buffer to IO
int ioWrite(IO io, char* buf, int size) {
	int written=0;
	io->e[0]='\0';
	if(io->type==SOCKSTREAM_TYPE) {
		// WinXP (at least) doesn't support write on a socket, so we use send instead
		written=send(io->s, buf, size,0);
	} else if(io->type==SOCKDGRAM_TYPE) {
		written=connWriteTo((Conn)io,((Conn)io)->remote,buf, size);
	} else if(io->type==FILE_TYPE) {
		written=write(io->s, buf, size);
	} else {
		newError(io->e,"Unsupported IO type!\n");
		return -1;
	}
	if(written<0) {
		Error e;
		newError(io->e,"Write error %s\n",ERRDESC(e));
	}
	return written;
}

// ioClose closes the IO
int ioClose(IO io) {
	int size=sizeOfType(io->type);
	if(closesocket(io->s)) {
		Error e;
		newError(io->e,"Close: %s\n",ERRDESC(e));
		return -1;
	}
	memset(io,size,0);
	free(io);
	return 0;
}
