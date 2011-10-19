/*  @brief The server application interface
 *  This interface starts a thread which continous waits for messages from the server application or from the client application.
 *  The developer has to derive this interface and reimplement the run function.
 *  Several objects can be registered to the interface for getting update by the corresponding observer.
 *
 *  @author Benedikt Lehnertz (ITWM Fraunhofer)
 *  Created on: April 20, 2010
 */

#ifndef SERVERAPPLICATIONINTERFACE_H_
#define SERVERAPPLICATIONINTERFACE_H_

#include <string>
#include <map>

#include "ApplicationHeader.hpp"
#include "CommunicationHandlerThread.h"
#include "Observer.h"

namespace PSProMigIF
{
  class ServerCommunication;

  class ServerApplicationInterface : public CommunicationHandlerThreadItem
  {
    public:

      /*! @brief Constructor
       *  @param _pServerCommunication Waiting on this communication channel for messages
       *  @param _application Identifies the application
       */
      ServerApplicationInterface(ServerCommunication* const _pServerCommunication, Applications _application);

      /*! @brief Destructor
       */
      virtual ~ServerApplicationInterface(void);

      /*! @brief Initializes the class with a communication channel
       *  @param _pServerCommunication Waiting on this communication channel for messages
       */
      void init(ServerCommunication* const _pServerCommunication);

      /*! @brief Add to the observer
       *  @param _sName Identifier name
       *  @param _pItem The object which will receive updates from the observer
       */
      void addToObserver(const std::string& _sName, ObserverItem* const _pItem);

      /*! @brief Remove from the observer
       *  @param _sName Identifier name
       */
      void removeFromObserver(const std::string& _sName);

      /*! @brief Starts the interface
       *  Starts a new thread which will receive messages
       */
      void start(void);

      /*! @brief The function which will be executed when a new message was received
       *  @param _pMessage The message
       */
      virtual void run(Message* const _pMessage) = 0;

      /*! @brief Stop the application interface
       *  The thread will be canceled
       */
      void stop(void);

      /*! @return Returns the application identifier
       */
      Applications application(void);

    protected:

      ServerCommunication* communication(void) const;

      /*! @return Returns the pointer to the observer
       */
      Observer* observer(void);

      virtual void stopRoutine(void);

    private:

      /*! @brief The thread object
       */
      CommunicationHandlerThread* m_pHandlerThread;

      /*! @brief The communication object
       */
      ServerCommunication* m_pServerCommunication;

      /*! @brief The observer
       */
      Observer mObserver;

      /*! @brief The application identifier
       */
      const Applications mApplication;
  };

}

#endif
