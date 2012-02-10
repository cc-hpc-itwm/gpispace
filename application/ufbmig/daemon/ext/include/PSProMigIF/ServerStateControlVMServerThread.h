#ifndef SERVERSTATECONTROLVMTHREAD_H_
#define SERVERSTATECONTROLVMTHREAD_H_

#include "SocketThread.h"

class PSProServerSocket;

namespace ServerCommunication
{
  class ServerStateControl;

  /*! @brief The class with the control thread
   */
  class ServerStateControlClientThread: public SocketThread
  {
    public:
      /*! @brief Constructor
       */
      ServerStateControlClientThread(ServerStateControl* const _pServerStateControl);

      /*! @brief Set a socket fd on which the server state control listens
       *  @param _fd The file descriptor
       */
      void setClientFd(int _fd);

    protected:

      /*! @brief Function for receiving data on the given fd
       *  @param _fd The file descriptor
       *  @return Returns true if receiving data else false
       */
      virtual bool receiveData(int _fd);

      int receiveServerData();

    private:

      int m_clientFd;

      ServerStateControl* const m_pServerStateControl;
  };

  class ServerStateControlVMServerThread: public SocketThread
  {
    public:
      ServerStateControlVMServerThread();
      void setClientFd(int _fd);
      int setListenSocket(PSProServerSocket* _pListenSocket);

    protected:
      virtual bool receiveData(int _fd);

      int receiveClientData();
      int receiveListenSocket();

    private:

      int m_clientFd;
      PSProServerSocket* m_pListenSocket;
  };
}

#endif

