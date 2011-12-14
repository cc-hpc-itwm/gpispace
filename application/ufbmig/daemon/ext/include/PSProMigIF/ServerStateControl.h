/*  @brief This file holds all instances which are necessary to control the connection between client and server
 *
 *  @author Benedikt Lehnertz (ITWM Fraunhofer)
 *  Created on: Mar 24, 2010
 */

#ifndef SERVERSTATECONTROL_H_
#define SERVERSTATECONTROL_H_

#include "CommunicationStructs.hpp"
#include "Exception.hpp"

namespace PSProMigIF
{
  class StartServer;
  class ServerStateControlThread;
  class ServerCommunication;

  class ServerStateControl
  {
    public:

      /*! @brief Typedef for ServerStateControl exception object
       */
      typedef Exception<ServerStateControlError> ServerStateControlException;

      /*! @brief Constructor
       */
      ServerStateControl(void);

      /* @brief Destructor; Destroys the ServerStateControl object
       */
      virtual ~ServerStateControl(void);

      /*! @return Returns the state of the server connection
       */
      const ServerState& serverState(void);

      /*! @return Returns the server stop reason
       */
      const ServerStopReason& serverStopReason(void);

      /*! @return Returns the working state of the server
       */
      const ServerWorkingState& serverWorkingState(void);

      /*! @brief Set the pointer to the server which started the state control
       *  @param _pServer The server
       */
      void setServer(StartServer* const _pServer);

      /*! @brief Pointer to the server which started the state control
       *  @return Pointer to the server
       */
      StartServer* server(void);

      /*! @brief Initialize the serverStateControl object
       *  @param _applicationInfo The server application info
       */
      virtual void init(const StartupInfo& _applicationInfo) throw (ServerStateControlException);

      /*! @brief Start the server control
       */
      virtual void start(void) throw (ServerStateControlException);

      /*! @brief Start the server control
       */
      virtual void busy(void) throw (ServerStateControlException);

      /*! @brief Start the server control
       */
      virtual void idle(void) throw (ServerStateControlException);

      /*! @brief Start the control thread which always controls the connection
       */
      virtual void startControlThread(void) throw (ServerStateControlException) = 0;

      /*! @brief Stops the server control
       */
      virtual void stop(void) throw (ServerStateControlException);

      /*! @brief This function is called when the server crashes
       */
      virtual void crash() throw (ServerStateControlException);

      /*! @brief Check before starting binary
       *  For example, checks if a binary is already running
       *  @param _applicationInfo Checks for the given application
       */
      virtual void check(StartupInfo& _applicationInfo) throw (ServerStateControlException) = 0;

      /*! @brief Generate error string on error codes received from the ServerStateControl class
       *  @param e The exception
       *  @return The error string
       */
      static std::string generateErrorString(const ServerStateControlException& _e);

      /*! @return Returns the application info
       */
      const StartupInfo& applicationInfo(void);

      /*! @return Returns the application info
       */
      const ConnectionInfo& connectionInfo(void);

      /*! @return Returns communication channel
       */
      ServerCommunication* communication(void) const;

      void updateServerState(ServerState _state);

    protected:

      /*! @brief Start the control socket
       *  @param _bServerInstance Used by server instance?
       */
      virtual void connect(void) throw (ServerStateControlException) = 0;

      /*! @brief The control socket
       */
      ServerCommunication* m_pControlCommunication;

      /*! @brief The control socket
       */
      ServerStateControlThread* m_pControlThread;

    private:

      /*! @brief Holds the state of the server connection
       */
      ServerState mServerState;

      /*! @brief Server stop reason
       */
      ServerStopReason mServerStopReason;

      /*! @brief Server working state
       */
      ServerWorkingState mServerWorkingState;

      /*! @brief Holds the information of the running application
       *  If there is already an application running on the selected host, the information from this application will be written in this struct.
       */
      StartupInfo mApplicationInfo;

      /*! @brief Connection info
       */
      ConnectionInfo mConnectionInfo;

      /*! @brief Pointer to the server which started the state control
       */
      StartServer* m_pServer;

      /*! @brief Mutex controls object access
       */
      pthread_mutex_t mMutex;
  };
}

#endif
