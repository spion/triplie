// Definition of the Socket class

#ifndef Socket_class
#define Socket_class


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>
#include <errno.h>

const int MAXHOSTNAME = 200;
const int MAXCONNECTIONS = 5;
const int MAXRECV = 1536;
#ifndef MSG_NOSIGNAL
const int MSG_NOSIGNAL = 0; // defined by dgame
#endif
class Socket
{
 public:
  Socket();
  virtual ~Socket();

  // Server initialization
  bool create();
  bool bind ( const int port );
  bool listen();
  bool accept ( Socket& );

  // Client initialization
  bool connect ( const std::string host, const int port );

  // Data Transimission
  bool send ( const std::string );
  int recv ( std::string& );


  void set_non_blocking ( const bool );
  void destroy();

  bool is_valid() const { return m_sock != -1; }

  int get_state() { return m_sockerr; }
 private:

  int m_sock;
  int m_sockerr;
  sockaddr_in m_addr;


};


#endif
