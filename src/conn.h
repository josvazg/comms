/**
	Conns interface symplifies and encapsulates Socket access and other streams types alike
    allowing you to use the same API for tcp and udp sockets, plain, SSL, etc

*/
typedef struct Conn_S * Conn;

#define MAX_ERROR_SIZE 256
#define MAX_ADDR_SIZE 256

// Error messages
typedef char Error[MAX_ERROR_SIZE];

// Addresses
typedef char Address[MAX_ADDR_SIZE];

// commsInit starts the communications (Needed due to Windows API restrictions)
void commsInit(Error err);

// Is this on error?, returns 0 when there is no error and !0 otherwise
int onError(Error err);

// connDial connects or prepares a communication on a network 'net' to address 'addr'
// On any error, connDial returns NULL and err will have some error message filled in 
Conn connDial(char* net, char* addr, Error err);

// connError returns the latest Conn's Error
// use onError(connError(conn)) to test for conn(ection) errors
Error* connError(Conn conn);

// Fills raddr with the remote connected (or last received data) address
// On any error the address is empty and Conn's Error is set
void connRemoteAddress(Conn conn, Address raaddr);

// connRead reads contents from conn to the given buffer buf (at most size bytes) and
// returns the number of bytes read OR -1 and Conn's Error is set
int connRead(Conn conn, char* buf, int size);

// connWrite writes contents from the buf buffer to Conn and
// returns the number of bytes written OR -1 and Conn's Error is set
int connWrite(Conn conn, char* buf, int size);

// connClose closes the Connection/Stream
// On success it should return 0 and conn is no longer points to valid data, SO DON'T USE IT AGAIN!
// On failure it returns a non zero value and Conn's Error is set
int connClose(Conn conn);
