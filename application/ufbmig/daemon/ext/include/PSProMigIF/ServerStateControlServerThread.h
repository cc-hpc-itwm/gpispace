/*! @brief The server state control thread (on server side)
 *  This thread listens on messages from the server state control on the other side
 *
 *  @author Benedikt Lehnertz (ITWM Fraunhofer)
 *   Created on: Mar 26, 2010
 *
 */

#ifndef SERVERSTATECONTROLVMTHREAD_H_
#define SERVERSTATECONTROLVMTHREAD_H_

#include "ServerStateControlThread.h"

class PSProServerSocket;
class ServerStateControl;

namespace PSProMigIF
{
  class ServerStateControl;

  /*! @brief The class with the control thread
   */
  class ServerStateControlServerThread: public ServerStateControlThread
  {
    public:
      /*! @brief The class with the control thread
       */
      ServerStateControlServerThread(ServerStateControl* const _pServerStateControl);

      /*! @brief Destructor
       */
      virtual ~ServerStateControlServerThread(void);

      /*! @brief Set the listen socket
       *  @param _pListenSocket The listen socket
       *  @return Returns a value smaller 0 if there was an error
       */
      int setListenSocket(PSProServerSocket* _pListenSocket);

    protected:

      /*! @brief Function for receiving data on the given fd
       *  @param _fd The file descriptor
       *  @return Returns true if receiving data else false
       */
      virtual bool receiveData(int _fd);

      /*! @brief If the server receives data from the client
       *  @return Returns a value smaller 0 if there was an error
       */
      int receiveClientData();

      /*! @brief If there was an request for connection this function will be called
       *  @return Returns a value smaller 0 if there was an error
       */
      int receiveListenSocket();

    private:

      /*! @brief The listen socket
       */
      PSProServerSocket* m_pListenSocket;
  };
}

#endif

