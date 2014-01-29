/*  @brief This file holds all instances which are necessary to communicate with the sever application
 *  The developer has to derive his own class from class ServerCommunication; this class must be registered with function register().
 *  To use the register function it is necessary to write for every derived ServerCommunication class a ServerCommunicationHelper
 *
 *  @author Benedikt Lehnertz (ITWM Fraunhofer)
 *  Created on: Mar 18, 2010
 */

#ifndef ServerCommunication_H_
#define ServerCommunication_H_

#include "CommunicationStructs.hpp"
#include "Exception.hpp"
#include "PSProSockets.h"

#include <map>
#include <string>

namespace PSProMigIF
{
  class ServerCommunication;
  struct Message;

  /*! @brief Helper class for generating derived classes of class ServerCommunication
   *  Every derived class from ServerCommunication has to derive this object, too.
   */
  class ServerCommunicationHelper
  {
    public:
      /*! @brief Create object of class ServerCommunication
       */
      virtual ServerCommunication* create();
      virtual ~ServerCommunicationHelper() = 0;
  };

  /*! @brief Enum which holds the error codes of the server communication
   */
  enum ServerCommunicationError
  {
    ServerCommunication_FeatureNotSupported = 1,
    ServerCommunication_ConnectionFailed = 2,
    ServerCommunication_CouldNotBindSocket = 3,
    ServerCommunication_ServerListenProblem = 4,
    ServerCommunication_ServerSocketNotBinded = 5,

    ServerCommunication_NoError = 255
  };

  /*! @brief Class for destroying the ServerCommunication object
   */
  class ServerCommunicationDestroyer
  {
    public:
      /*! @brief Constructor
       *  @param _sKey Key for instance
       *  @param _pInstance Instance which will be destroyed
       */
      ServerCommunicationDestroyer(void);

      /*! @brief Destroys the Destroyer-object and so the StartServer-object
       */
      ~ServerCommunicationDestroyer(void);

      /*! @brief Insert new ServerCommunication-instance to the destroyer object
       *  @param _pInstance The instance
       */
      void insert(std::pair<std::string, ServerCommunication*> _pInstance);

    private:
      /*! @brief Start server destroyer
       */
      std::map<std::string, ServerCommunication*> mSingletonInstances;
  };

  /*! @brief Class for communication between client and server
   */
  class ServerCommunication
  {
      friend class ServerCommunicationDestroyer;

    public:
      /*! @brief Typedef for StartServer exception object
       */
      typedef Exception<ServerCommunicationError> ServerCommunicationException;

      /*! @brief Constructor
       */
      ServerCommunication(void);

      /*! @brief Destructor; Destroys the communication object
       */
      virtual ~ServerCommunication(void);

      /*! @brief Register the derived classes
       *  @param _sName Name of the registered instance
       *  @param _pInstance Pointer to the instance
       */
      static void registerInstance(const std::string& _sName, ServerCommunicationHelper* const _pHelper);

      /*! @brief Get current instance of the ServerCommunication
       *  @return The pointer to the current instance
       */
      static ServerCommunication* getInstance(const std::string& _sName);

      /*! @brief Initialize the communication object
       *  @param _sHostname The hostname to which the connections will be build
       */
      void init(const ConnectionInfo& _connectionInfo=ConnectionInfo()) throw (ServerCommunicationException);

      /*! @brief Initialize the connection between client and backend
       *  Connect to any host
       *  @param _bListen If true this instance will listen for a connection
       */
      virtual void connect(void) throw (ServerCommunicationException);

      /*! @brief Close the connection
       */
      virtual void close(void) throw (ServerCommunicationException);

      /*! @return Returns the information about the connection
       */
      const ConnectionInfo& connectionInfo(void);

      /*! @brief Generate error messages
       */
      static std::string generateErrorString(const ServerCommunicationException& _e);

      /*! @return Pointer to the socket
       */
      PSProSocket* socket(void);

      /*! @brief Function for sending a message over this communication object
       *  @param _pMessage The message
       */
      void sendMsg(Message* const pMessage, const int _nTimeout=-1);

      /*! @brief Function for receiving a message over this communication object
       *  @return The received message
       */
      Message* recvMsg(void);

    protected:

      /*! @brief This routine is called to initialize the communication instance
       */
      virtual void initRoutine(void) throw (ServerCommunicationException);

    private:

      /*! @brief Singleton destroyer
       */
      static ServerCommunicationDestroyer mSingletonDestroyer;

      /*! @brief Singleton-class pointer
       */
      static std::map<std::string, ServerCommunication*> mInstance;

      /*! @brief Holds information about the connection
       */
      ConnectionInfo mConnectionInfo;

      /*! @brief The socket connection
       */
      PSProSocket mSocket;

      /*! @brief Mutex controls object access
       */
      pthread_mutex_t mMutex;
  };

  /*! @brief Helper class for generating derived classes of class ServerCommunicationListen
   *  Every derived class from ServerCommunication has to derive this object, too.
   */
  class ServerCommunicationListenHelper: public ServerCommunicationHelper
  {
    public:
      /*! @brief Create object of class ServerCommunication
       */
      virtual ServerCommunication* create();
      virtual ~ServerCommunicationListenHelper() = 0;
  };

  /*! @brief Class for communication between client and server
   */
  class ServerCommunicationListen: public ServerCommunication
  {
      friend class ServerCommunicationDestroyer;

    public:

      /*! @brief Constructor
       */
      ServerCommunicationListen(void);

      /*! @brief Destructor; Destroys the communication object
       */
      virtual ~ServerCommunicationListen(void);

      /*! @brief Initialize the connection between client and backend
       *  Connect to any host
       *  @param _bListen If true this instance will listen for a connection
       */
      virtual void connect(void) throw (ServerCommunicationException);

      /*! @brief Close the connection
       */
      virtual void close(void) throw (ServerCommunicationException);

      /*! @return Pointer to the socket
       *  @param _uPort Search server socket for the given port
       */
      PSProServerSocket* serverSocket(unsigned int _uPort);

    protected:

      /*! @brief This routine is called to initialize the communication instance
       */
      virtual void initRoutine(void) throw (ServerCommunicationException);

    private:

      /*! @brief The server socket connection
       */
      static std::map<int, PSProServerSocket*> mServerSocket;
  };
}

#endif
