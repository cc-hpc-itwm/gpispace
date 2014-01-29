#ifndef PSPRO_SOCKETS_H
#define PSPRO_SOCKETS_H

#include <stdint.h>
#include <pthread.h>
#include <netinet/in.h>

#include <string>

#define ERROR_SOCKET_ALREADY_ACTIVE -1
#define ERROR_SOCKET_INACTIVE       -2
#define ERROR_SOCKET_NETWORK        -3
#define ERROR_SOCKET_SYSTEM         -4
#define ERROR_SOCKET_UNKNOWN_HOST   -5
#define ERROR_SOCKET_TIMEOUT        -6
#define ERROR_SOCKET_CLOSED         -7
#define ERROR_SOCKET_BAD_ARGUMENT   -8
#define ERROR_SOCKET_UNDEFINED      -9

class PSProSocketException
{
  public:
    PSProSocketException(int _e):m_error(_e)         {}
    int getError() const                             {return m_error;}
  private:
    int m_error;
};


class PSProSocket
{
  public:
    PSProSocket();
    virtual ~PSProSocket();
    //int frontend_connect(const char *hn,const int port,const unsigned int timeout_secs);
    int connect(const char *_hostname, const unsigned int _port , const int _timeout_secs, bool _bMilliseconds = false);
    
    int setConnected(int _fd, const struct sockaddr_in* _pClientAddress);
    
    virtual void close();

    /// This one replaces rem_recv_buffer2(), but with proper error handling
    /// return codes: <0 failure, 0 success
    int recvBuffer(char* _buffer, const int _len, const int _timeout_sec = -1);

    /// This one replaces rem_recv_cmd(), but with proper error handling
    int recvCmd(int32_t& _rCmd, const int _timeout_secs = -1);

    int sendBuffer(const char* _buffer, const int _len, const int _timeout_sec = -1);

    int sendCmd(const int32_t _cmd, const int _timeout_sec = -1);

    /// Same as rif::rem_recv_cmd(), in case of error an exception is thrown
    int32_t rem_recv_cmd() throw(PSProSocketException);

    /// Same as rif::rem_recv_buffer(), in case of error an exception is thrown
    void rem_recv_buffer(char* _buffer,const int _len) throw(PSProSocketException);

    /// Same as rif::rem_send_cmd(), in case of error an exception is thrown
    void rem_send_cmd(const int32_t _cmd) throw (PSProSocketException);

    /// Same as rif::rem_send_buffer(), in case of error an exception is thrown
    void rem_send_buffer(const char* _buffer, const int _len) throw (PSProSocketException);

    int32_t getRemoteIpAddress() const;

    const char* getRemoteHostname() const;

    int getFd() const;

    bool isConnected() const;

    static void setSocketOptions(int _fd);

    inline unsigned long writtenBytes(void) const
    {
      return m_ulWrittenBytes;
    }

  protected:
    int m_socketFd;
    int32_t m_remoteIpAddress;
    std::string m_remoteHostname;
    mutable pthread_mutex_t m_socketMutex;
    mutable pthread_mutex_t m_recvMutex;
    mutable pthread_mutex_t m_sendMutex;
    unsigned long m_ulWrittenBytes;
};


/** The PSProServerSocket class implements a TCP server socket, which can accept new TCP connections on a 
 *  given port (via bindSocket() + waitForConnection().
 *
 * Attention: this class is not thread-safe, it should  be handled by just one thread in some mainloop-like code.
 */
class PSProServerSocket
{
  public:
    /// Creates a new PSProServerSocket instance. bindSocket() must be called afterwards to make it accept connections.
    PSProServerSocket();
    ~PSProServerSocket();

    /** This is basically socket() + bind() + listen(). It sets up a TCP server socket on the given port, so 
     *  new connections can be received via waitForConnection(). Call close() to close() it again. */
    int bindSocket(unsigned int _port);

    /// This is basically select() + accept()
    int waitForConnection(int* _pFd, struct sockaddr_in* _pClientAddress, const int _timeout_sec);

    /** Waits for the given timeout for a new connection. bindSocket() must have been called successfully before.
     *  If there is a new connection, \a _pClientSocket is set up accordingly, so this PSProSocket can be used
     *  afterwards to communicate over this new connection. */
    int waitForConnection(PSProSocket* _pClientSocket, const int _timeout_sec);

    /** Closes the server socket, i.e. after that it cannot accept new connections anymore. 
     *  To enable it again, bindSocket() has to be called again. */
    void close();

    /// Returns true if the server socket is in listen state, i.e. ready to accept connections.
    bool isActive() const        {return (m_listenFd != -1);}

    /// Returns the file descriptor of the server socket.
    int getFd() const            {return m_listenFd;}

  protected:
    int m_listenFd;
};


#endif
