/*  @brief This file holds all instances which are necessary to control the connection between client and server (here for VM)
 *
 *  @author Benedikt Lehnertz (ITWM Fraunhofer)
 *  Created on: Mar 26, 2010
 */

#ifndef SERVERSTATECONTROLVMSERVER_H_
#define SERVERSTATECONTROLVMSERVER_H_

#include "ServerStateControl.h"

namespace PSProMigIF
{
  class ServerStateControlServer: public ServerStateControl
  {
    public:
      /*! @brief Constructor
       */
      ServerStateControlServer(void);

      /*! @brief Destructor
       */
      ~ServerStateControlServer(void);

      /*! @brief Start the server control
       */
      virtual void busy(void) throw (ServerStateControlException);

      /*! @brief Start the server control
       */
      virtual void idle(void) throw (ServerStateControlException);

    protected:

      /*! @brief Start the control socket
       *  @param _bServerInstance Used by server instance?
       */
      void connect() throw (ServerStateControlException);

      /*! @brief Check before starting binary
       *  For example, checks if a binary is already running
       *  @param _applicationInfo Checks for the given application
       */
      virtual void check(StartupInfo& _applicationInfo) throw (ServerStateControlException);

      /*! @brief Start the control thread which always controls the connection
       */
      virtual void startControlThread(void) throw (ServerStateControlException);

      /*! @brief Stops the server control
       */
      void stop(void) throw (ServerStateControlException);
  };
}

#endif
