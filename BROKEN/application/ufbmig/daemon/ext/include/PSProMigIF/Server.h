/*  @brief This file the class which starts server applications
 *  This includes the server application of server side; call this only on backend side
 *
 *  @author Benedikt Lehnertz (ITWM Fraunhofer)
 *  Created on: Mar 23, 2010
 */

#ifndef SERVER_H_
#define SERVER_H_

#include "StartServer.h"

namespace PSProMigIF
{
  class StartServer;

  /*! @brief Helper class for generating derived classes of class StartServer (here: VM Server)
   *  Every derived class from StartServer has to derive this object, too.
   */
  class ServerHelper: public StartServerHelper
  {
    public:

      ServerHelper(const std::string& _sVersion);

      /*! @brief Create object of class Server
       */
      virtual StartServer* create(void);

      /*! @brief Create ServerStateControl object
       */
      virtual ServerStateControl* createStateControl(void);
  };

  /*! @brief class for starting the server application (here: VM Server)
   */
  class Server: public StartServer
  {
      friend class ServerHelper;

    protected:
      /*! @brief Constructor
       */
      Server(const std::string& _sVersion);

      /*! @brief Destructor; Destroys the Server object
       */
      virtual ~Server(void);

      /*! @brief Start routine
       */
      virtual void startRoutine(void) throw (StartServerException);

      /*! @brief Stop routine
       */
      virtual void stopRoutine(void) throw (StartServerException);

      /*! @brief Disconnect routine
       */
      virtual void disconnectRoutine(void) throw (StartServerException);

      /*! @brief Reconnect routine
       */
      virtual void reconnectRoutine(void) throw (StartServerException);

      /*! @brief Idle routine
       */
      virtual void idleRoutine(void) throw (StartServerException);

      /*! @brief Busy routine
       */
      virtual void busyRoutine(void) throw (StartServerException);

      /*! @brief Crash routine
       */
      virtual void crashRoutine(void) throw (StartServerException);

  };
}



#endif
