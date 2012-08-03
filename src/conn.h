/**
	Conn interface symplifies and encapsulates Socket access and other streams types alike
    allowing you to use the same API for tcp and udp sockets, plain, SSL, etc

*/
typedef struct Conn_S * Conn;

#define MAX_ERROR_SIZE 256
#define MAX_ADDR_SIZE 512

// Error messages
typedef char Error[MAX_ERROR_SIZE];

// Addresses
typedef char Address[MAX_ADDR_SIZE];

// commsInit starts the communications (Needed due to Windows API restrictions)
void commsInit(Error err);

// Is this on error?, returns 0 when there is no error and !0 otherwise
int onError(Error err);

// ConnDial connects or prepares a communication on a network 'net' to address 'addr'
// err is an error placeholder, it must be checked afterwards
Conn ConnDial(char* net, char* addr, Error err);

// ConnError returns the latest Conn's Error
Error* ConnError(Conn conn);

// Fills 'ra' with the remote connected address
void ConnRemoteAddress(Conn conn, Address ra);

// onConnError is a shortcut for onError(ConnError(conn))
int onConnError(Conn conn);

// ConnRead reads contents from Conn to the given buffer and
// returns the number of bytes read OR -1 and Conn's Error is set
int ConnRead(Conn conn, char* buf, int size);

// ConnWrite writes contents from the buf buffer to Conn and
// returns the number of bytes written OR -1 and Conn's Error is set
int ConnWrite(Conn conn, char* buf, int size);

// ConnClose closes the Connection/Stream
// On success it should return 0
// On failure it returns a non zero value and Conn Error is set
int ConnClose(Conn conn);
