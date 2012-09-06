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


