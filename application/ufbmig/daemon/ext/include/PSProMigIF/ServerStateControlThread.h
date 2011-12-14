/*! @brief The server state control thread
 *  This thread listens on messages from the server state control on the other side
 *
 *  @author Benedikt Lehnertz (ITWM Fraunhofer)
 *   Created on: Mar 26, 2010
 *
 */

#ifndef SERVERSTATECONTROLTHREAD_H_
#define SERVERSTATECONTROLTHREAD_H_

#include "SocketThread.h"

class PSProServerSocket;

namespace PSProMigIF
{
  class ServerStateControl;

  /*! @brief The class with the control thread
   */
  class ServerStateControlThread: public SocketThread
  {
    public:
      /*! @brief Constructor
       *  @param _pServerStateControl Pointer to the server state control
       */
      ServerStateControlThread(ServerStateControl* const _pServerStateControl);

      /*! @brief Destructor
       */
      virtual ~ServerStateControlThread(void);

      /*! @brief Set a socket fd on which the server state control listens
       *  @param _fd The file descriptor
       */
      void setClientFd(int _fd);

    protected:

      /*! @brief Function for receiving data on the given fd
       *  @param _fd The file descriptor
       *  @return Returns true if receiving data else false
       */
      virtual bool receiveData(int _fd) = 0;

      /*! @brief File descriptor of socket connection
       */
      int m_clientFd;

      /*! @brief Pointer to the server state control
       */
      ServerStateControl* const m_pServerStateControl;
  };
}

#endif
