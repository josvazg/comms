/**
	Conns interface symplifies and encapsulates Socket access and other streams types alike
    allowing you to use the same API for tcp and udp sockets, plain, SSL, etc

*/
#ifdef _WIN32
  #define COMMS_API __declspec(dllexport)
#else
  #define COMMS_API
#endif

typedef struct Conn_S * Conn;

#define MAX_ERROR_SIZE 256
#define MAX_ADDR_SIZE 256

// Error messages
typedef char Error[MAX_ERROR_SIZE];

// Addresses
typedef char Address[MAX_ADDR_SIZE];

// commsInit starts the communications (Needed due to Windows API restrictions)
// On error a non zero value is returned and err Error's message is set
COMMS_API int commsInit(Error err);

// Is this on error?, returns 0 when there is no error and !0 otherwise
COMMS_API int onError(Error err);

// connDial connects or prepares a communication on a network 'net' to address 'addr'
// On any error, connDial returns NULL and err will have some error message filled in 
COMMS_API Conn connDial(char* net, char* addr, Error err);

// connError returns the latest Conn's Error
// use onError(connError(conn)) to test for conn(ection) errors
COMMS_API Error* connError(Conn conn);

// Fills raddr with the remote connected (or last received data) address
// On any error the address is empty and Conn's Error is set
COMMS_API void connRemoteAddress(Conn conn, Address raaddr);

// connRead reads contents from conn to the given buffer buf (at most size bytes) and
// returns the number of bytes read OR -1 and Conn's Error is set
COMMS_API int connRead(Conn conn, char* buf, int size);

// connWrite writes contents from the buf buffer to Conn and
// returns the number of bytes written OR -1 and Conn's Error is set
COMMS_API int connWrite(Conn conn, char* buf, int size);

// connClose closes the Connection/Stream
// On success it should return 0 and conn is no longer points to valid data, SO DON'T USE IT AGAIN!
// On failure it returns a non zero value and Conn's Error is set
COMMS_API int connClose(Conn conn);
