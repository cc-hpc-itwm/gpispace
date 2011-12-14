/*! @brief Just a simple communication handler
 *  When getting events (a new message) on a given socket a run()-function of a given object will be executed
 *
 *  @author Benedikt Lehnertz (ITWM Fraunhofer)
 *  Created on: April 21, 2010
 */

#ifndef COMMUNICATIONHANDLERTHREAD_H_
#define COMMUNICATIONHANDLERTHREAD_H_

#include "SocketThread.h"

namespace PSProMigIF
{
  class Message;

  /*! @brief The object which will be executed by the communication handler thread
   */
  class CommunicationHandlerThreadItem
  {
      friend class CommunicationHandlerThread;

    public:

      /*! @brief Constructor
       */
      CommunicationHandlerThreadItem(void);

    protected:

      /*! @brief run()-function which will be executed when are events on the socket of the communication handler thread
       *  @param _pMessage The message which was received
       */
      virtual void run(Message* const _pMessage) = 0;

  };

  /*! @brief This thread listens on a given communication channel and executes the run()-function of a given object
   */
  class CommunicationHandlerThread: public SocketThread
  {
    public:

      /*! @brief Constructor
       *  @param _pCommunicationHandlerThreadItem The run()-function if this object will be executed
       *  @param _nFd The file descriptor of the socket which will be used for getting messages
       */
      CommunicationHandlerThread(CommunicationHandlerThreadItem* const _pCommunicationHandlerThreadItem, int _nFd);

      /*! @brief Function for settings the file descriptor
       *  @param _nfd The file descriptor
       */
      void setClientFd(int _fd);

    protected:

      /*! @brief This function will be executed when there are new data on the given socket
       *  @param _nFd Socket's file descriptor
       *  @return True, if data were received successful
       */
      virtual bool receiveData(int _nFd);

    private:

      /*! @brief The run()-function if this object will be executed
       */
      CommunicationHandlerThreadItem* const m_pCommunicationHandlerThreadItem;

      /*! @brief Socket's file descriptor
       */
      int m_nClientFd;
  };
}

#endif
