/**
	Conns interface symplifies and encapsulates Socket access and other streams types alike
    allowing you to use the same API for tcp and udp sockets, plain, SSL, etc

*/
#ifdef _WIN32
  #define COMMS_API __declspec(dllexport)
#else
  #define COMMS_API
#endif

typedef struct IO_S * IO;

typedef struct Conn_S * Conn;

typedef struct Serv_S * Serv;

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
// addr must be on the form "<address>:seviceOrPort" 
// like ":80", "10.23.23.4:ftp" or "www.site.com:http"
// when no address is specified connDial assumes "localhost"
// On any error, connDial returns NULL and err will have some error message filled in 
COMMS_API Conn connDial(char* net, char* addr, Error err);

// connError returns the latest Conn's Error
// use onError(connError(conn)) to test for conn(ection) errors
COMMS_API Error* connError(Conn conn);

// connAddress fills addr with the local address
// On any error the address is empty and Conn's Error is set
COMMS_API void connAddress(Conn conn, Address addr);

// connRemoteAddress fills raddr with the remote connected (or last received data) address
// On any error the address is empty and Conn's Error is set
COMMS_API void connRemoteAddress(Conn conn, Address raddr);

// connRead reads contents from conn to the given buffer buf (at most size bytes) and
// returns the number of bytes read OR -1 and Conn's Error is set
// connRead NEVER changes the conn remote address, not even WITH UDP sockets
// if you need to know which UDP client you readed from, use connReadFrom instead
COMMS_API int connRead(Conn conn, char* buf, int size);

// connWrite writes contents from the buf buffer to Conn and
// returns the number of bytes written OR -1 and Conn's Error is set
// connWrite ALLWAYS sends to the conn remote address, even WITH UDP sockets
// if you need to send to a different UDP recipient useryou readed from, use connReadFrom instead
COMMS_API int connWrite(Conn conn, char* buf, int size);

// connListenMsgs creates a new datagram listener on a network 'net' (like 'udp') and address 'addr'
// addr must be on the form "<address>:seviceOrPort" just like in connDial
// when no address is specified servNew assumes "0.0.0.0" 
// please use specific addresses for IPv6 such as [::0]:80
// On any error, servNew returns NULL and err will have some error message filled in 
COMMS_API Conn connListenMsgs(char* net, char* addr, Error err);

// connReadFrom reads contents from a remote UDP source
// returns the number of bytes read and the source from address filled OR -1 and Conn's Error is set
COMMS_API int connReadFrom(Conn conn, Address from, char* buf, int size);

// connWriteTo writes contents of the buf buffer to the given remote UDP address 'to' and
// returns the number of bytes written OR -1 and Conn's Error is set
COMMS_API int connWriteTo(Conn conn, Address to, char* buf, int size);

// connClose closes the Connection/Stream
// On success it should return 0 and conn is no longer points to valid data, SO DON'T USE IT AGAIN!
// On failure it returns a non zero value and Conn's Error is set
COMMS_API int connClose(Conn conn);

// servNew creates a new server on a network 'net' and address 'addr'
// addr must be on the form "<address>:seviceOrPort" just like in connDial
// when no address is specified servNew assumes "0.0.0.0" 
// please use specific addresses for IPv6 such as [::0]:80
// On any error, servNew returns NULL and err will have some error message filled in 
COMMS_API Serv servNew(char* net, char* addr, Error err);

// servError returns the latest Serv's Error
// use onError(ServError(conn)) to test for server errors
COMMS_API Error* servError(Serv Serv);

// servAddress fills addr where the server is listening on
// On any error, servListen returns NULL and Serv's Error is set
COMMS_API void servAddress(Serv serv, Address addr);

// servAccept recives and accepts new connections incoming to Server serv
// On any error, servAccept returns NULL and Serv's Error is set
COMMS_API Conn servAccept(Serv serv);

// servClose closes the Server
// On success it should return 0 and serv is no longer points to valid data, SO DON'T USE IT AGAIN!
// On failure it returns a non zero value and Serv's Error is set
COMMS_API int servClose(Serv Serv);

// ioError returns the latest IO Error
// use onError(ioError(req)) to test for IO errors
COMMS_API Error* ioError(IO io);

// ioRead reads contents from io to the given buffer buf (at most size bytes) and
// returns the number of bytes read OR -1 and IO's Error is set
COMMS_API int ioRead(IO io, char* buf, int size);

// ioWrite writes contents from the buf buffer to IO and
// returns the number of bytes written OR -1 and IO's Error is set
COMMS_API int ioWrite(IO io, char* buf, int size);

// ioClose closes the IO
// On success it should return 0 and io no longer points to valid data, SO DON'T USE IT AGAIN!
// On failure it returns a non zero value and IO's Error is set
COMMS_API int ioClose(IO io);
