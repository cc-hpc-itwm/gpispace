/*  @brief This file the class which starts server applications
 *  This includes the startup of the server application
 *
 *  @author Benedikt Lehnertz (ITWM Fraunhofer)
 *  Created on: Mar 18, 2010
 */

#ifndef STARTSERVER_H_
#define STARTSERVER_H_

#include "Exception.hpp"
#include "CommunicationStructs.hpp"

#include <cstdio>
#include <map>
#include <string>

#include <pthread.h>

namespace PSProMigIF
{
  class StartServer;
  class ServerStateControl;
  class ServerApplicationInterface;
  class ServerCommunication;

  class StartServerRememberPassword
  {
    public:

      static StartServerRememberPassword* getInstance();

      void add(const std::string& _user, const std::string& _password);

      std::string request(const std::string& _user);

    private:

      StartServerRememberPassword(void);

      ~StartServerRememberPassword(void);

      char* m_pPasswords;

      char* m_pUser;

      static const unsigned int s_uMapSize = 5;

      static const unsigned int s_uMapEntrySize = 64;

      static StartServerRememberPassword* m_pInstance;

      unsigned int m_uWritePos;

      bool m_bUsePasswortRemember;
  };


  /*! @brief Class for destroying the start server object
   */
  class StartServerDestroyer
  {
    public:
      /*! @brief Constructor
       *  @param _sKey Key for instance
       *  @param _pInstance Instance which will be destroyed
       */
      StartServerDestroyer(const std::string& _sKey = std::string(), StartServer* const _pSingleton = NULL);

      /*! @brief Destroys the Destroyer-object and so the StartServer-object
       */
      ~StartServerDestroyer(void);

      /*! @brief Set the instance which will be destroyed
       *  @param _sKey Key for instance
       *  @param _pSingleton Instance which will be destroyed
       */
      void insert(const std::string& _sKey, StartServer* const _pSingleton);

    private:

      /*! @brief Start server destroyer
       */
      std::map<std::string, StartServer*> mSingletonInstances;
  };

  /*! @brief Helper class for generating derived classes of class StartServer
   *  Every derived class from StartServer has to derive this object, too.
   */
  class StartServerHelper
  {
    public:

      StartServerHelper(const std::string& _sVersion);

      virtual ~StartServerHelper(void);

      /*! @brief Create object of class StartServer
       */
      virtual StartServer* create(void) = 0;

      /*! @brief Create ServerStateControl object
       */
      virtual ServerStateControl* createStateControl(void) = 0;

      /*! @return Returns the version of the application server
       */
      std::string version(void) const;

    private:

      const std::string m_sVersion;
  };

  /*! @brief class for starting the server application
   */
  class StartServer
  {
      friend class StartServerDestroyer;

    public:

      /*! @brief Add an application interface to the server
       *  @param _pApplicationIF The application interface
       */
      void addServerApplicationInterface(ServerApplicationInterface* const _pApplicationIF);

      /*! @brief Add a server communication to the server
       *  @param _pApplicationIF The communication
       */
      void addCommunication(ServerCommunication* const _pServerCommunication);

      /*! @return The communication
       */
      ServerCommunication* communication(void);

      /*! @return The application interface
       */
      ServerApplicationInterface* applicationIF(void);

      /*! @return Returns the state of the server connection
       */
      const ServerState& serverState(void);

      /*! @return Returns the server stop reason
       */
      const ServerStopReason& serverStopReason(void);

      /*! @return Returns the working state of the server
       */
      const ServerWorkingState& serverWorkingState(void);

      /*! @brief Typedef for StartServer exception object
       */
      typedef Exception<StartServerError> StartServerException;

      /*! @brief Get current instance of the StartServer object
       *  @return The pointer to the current instance
       */
      static StartServer* getInstance(const std::string& _sName);

      /*! @brief Register the derived classes
       *  @param _sName Name of the registered instance
       *  @param _pInstance Pointer to the instance
       */
      static void registerInstance(const std::string& _sName, StartServerHelper* const _pInstance);

      /*! @brief Initialize the StartServer instance
       *  @param _pServerCommunication The server communication instance; holds the server connection instances
       *  @param _startupInfo Holds every information which is necessary to start the server application
       */
      virtual void init(const StartupInfo& _startupInfo = StartupInfo()) throw (StartServerException);

      /*! @brief Start server application
       *  @param _bKillAlreadyRunningApplications
       */
      void start(bool _bKillAlreadyRunningApplications = false) throw (StartServerException);

      /*! @brief Disconnect from server application
       *  Server is still alive
       */
      void disconnect(void) throw (StartServerException);

      /*! @brief Reconnect to already running server application
       */
      void reconnect(void) throw (StartServerException);

      /*! @brief Stop server application
       */
      void stop(void) throw (StartServerException);

      /*! @brief Stop server application
       */
      static void stopAll(void);

      /*! @brief Will be called when there was a crash (on the server)
       */
      void crash(void) throw (StartServerException);

      /*! @brief Stop server application
       */
      void idle(void) throw (StartServerException);

      /*! @brief Stop server application
       */
      void busy(void) throw (StartServerException);

      /*! @return Name of the server in the map
       */
      std::string mapname(void);

      /*! @brief Returns the information of the running application
       *  In the case that there is already an application running on the selected that information will returned
       *  @param _bKillAlreadyRunningApplications If this parameter is set to true already running applications will be killed
       */
      const StartupInfo& applicationInfo(void);

      /*! @brief Returns the start information for the serer
       */
      const StartupInfo& startupInfo(void);

      /*! @brief Set an application interface which will be administrated by the start server class
       *  @param _pInterface The interface
       */
      void setApplicationIF(ServerApplicationInterface* const _pInterface);

      ServerApplicationInterface* applicationIF(void) const;

      /*! @brief Returns true if already running application will be killed by startup
       */
      bool killAlreadyRunningApplication(void);

      /*! @brief Cleans up the startupInfo struct
       *  For example, removes double "//" from the hostname and son on
       *  @param _startupInfo The startup info which will be cleaned up; the structure will be updated
       */
      static void cleanUpStartupInfo(StartupInfo& _startupInfo);

      /*! @brief Generate error string on error codes received from the startServer class
       *  @param e The exception
       *  @return The error string
       */
      static std::string generateErrorString(const StartServerException& _e);

      /*! @return Application version
       */
      std::string myVersion(void) const;

      /*! @return Application version
       */
      std::string remoteVersion(void) const;

      bool useRemoteFileEngine(void) const;

      void enableStateControl(bool _bEnable);

      /*! @brief If you enable this option; exception handling is done by the library
       *  @param _bEnable Enable / disable exception handline
       */
      void handleExceptionsByLibrary(bool _bEnable);

      bool handleExceptionsByLibrary(void) const;

    protected:

      /*! @brief Constructor
       */
      StartServer(const std::string& _sVersion);

      /*! @brief Destructor; Destroys the startServer object
       */
      virtual ~StartServer(void);

      /*! @brief Start routine
       */
      virtual void startRoutine(void) throw (StartServerException) = 0;

      /*! @brief Disconnect routine
       */
      virtual void disconnectRoutine(void) throw (StartServerException) = 0;

      /*! @brief Reconnect routine
       */
      virtual void reconnectRoutine(void) throw (StartServerException) = 0;

      /*! @brief Stop routine
       */
      virtual void stopRoutine(void) throw (StartServerException) = 0;

      /*! @brief Idle routine
       */
      virtual void idleRoutine(void) throw (StartServerException) = 0;

      /*! @brief Busy routine
       */
      virtual void busyRoutine(void) throw (StartServerException) = 0;

      /*! @brief Crash routine
       */
      virtual void crashRoutine(void) throw (StartServerException) = 0;

      /*! @brief Holds the information of the running application
       *  If there is already an application running on the selected host, the information from this application will be written in this struct.
       */
      StartupInfo mApplicationInfo;

      /*! @brief The server state control; control the connection between client and server
       */
      ServerStateControl* m_pServerStateControl;

      bool m_bUseRemoteFileEngine;

    private:

      void exchangeVersions(void);

      void checkServerNames(void) throw (StartServerException);

      void startStateControl(void) throw (StartServerException);

      void startStateControlThread(void) throw (StartServerException);

      /*! @brief Singleton-class pointer
       */
      static std::map<std::string, StartServer*> mInstance;

      static bool s_bObjectsAlreadyRegistered;

      /*! @brief Holds the registered class
       */
      static std::map<std::string, StartServerHelper*> mRegistratur;

      /*! @brief Destroyer
       */
      static StartServerDestroyer mStartServerDestroyer;

      /*! @brief Startup information
       */
      StartupInfo mStartupInfo;

      /*! @brief If this variable is set to true already running applications will be killed
       */
      bool m_bKillAlreadyRunningApplications;

      /*! @brief Is the server state control started?
       */
      bool m_bServerStateControlStarted;

      /*! @brief Is the server state control thread started?
       */
      bool m_bServerStateControlThreadStarted;

      /*! @brief Mutex which controls access to the StartServer object
       */
      pthread_mutex_t mMutex;

      /*! @brief The application interface which will be administrated by the start server class
       */
      ServerApplicationInterface* m_pApplicationIF;

      /*! @brief Server communication thread; for communication between client and server
       */
      ServerCommunication* m_pServerCommunication;

      /*! @brief Name in map
       */
      std::string m_sMapName;

      /*! @brief The version of the server application
       */
      const std::string m_sMyVersion;

      std::string m_sRemoteVersion;

      bool m_bUseStateControl;

      bool m_bHandleExceptionsByLibrary;
  };
}

#endif
